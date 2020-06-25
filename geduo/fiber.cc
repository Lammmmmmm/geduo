/*
 * @Author: choubin
 * @Date: 2020-06-14 22:31:08
 * @LastEditors: Choubin
 * @LastEditTime: 2020-06-26 00:56:03
 * @Description: file content
 */ 
#include <atomic>

#include "fiber.h"
#include "config.h"
#include "macro.h"
#include "log.h"
#include "scheduler.h"

namespace geduo {

static Logger::ptr g_logger = GEDUO_LOG_NAME("system");

static std::atomic<uint64_t> s_fiber_id {0};
static std::atomic<uint64_t> s_fiber_count {0};

static thread_local Fiber* t_fiber = nullptr;
static thread_local Fiber::ptr t_threadFiber = nullptr;

static ConfigVar<uint32_t>::ptr g_fiber_stack_size = 
    Config::Lookup<uint32_t>("fiber.stack_size", 128 * 1024, "fiber stack size");

class MallocStackAllocator {
public:
    static void* Alloc(size_t size) {
        return malloc(size);
    }

    static void Dealloc(void* vp, size_t size) {
        return free(vp);
    }
};

using StackAllocator = MallocStackAllocator;

uint64_t Fiber::GetFiberId() {
    if(t_fiber) return t_fiber->getId();
    return 0;
}

Fiber::Fiber() {
    m_state = EXEC;
    SetThis(this);

    if(getcontext(&m_ctx)) GEDUO_ASSERT2(false, "getcontext");

    ++s_fiber_count;

    GEDUO_LOG_DEBUG(g_logger) << "Fiber::Fiber main";
}

Fiber::Fiber(std::function<void()> cb, size_t stacksize, bool use_caller)
    :m_id(++s_fiber_id), m_cb(cb) {
    ++s_fiber_count;
    m_stacksize = stacksize ? stacksize : g_fiber_stack_size->getValue();

    m_stack = StackAllocator::Alloc(m_stacksize);
    if(getcontext(&m_ctx)) GEDUO_ASSERT2(false, "getcontext");
    m_ctx.uc_link = nullptr;
    m_ctx.uc_stack.ss_sp = m_stack;
    m_ctx.uc_stack.ss_size = m_stacksize;

    if(!use_caller) {
        makecontext(&m_ctx, &Fiber::MainFunc, 0);
    } else {
        makecontext(&m_ctx, &Fiber::CallerMainFunc, 0);
    }

    GEDUO_LOG_DEBUG(g_logger) << "Fiber::Fiber id = " << m_id;
}

Fiber::~Fiber() {
    --s_fiber_count;
    if(m_stack) {
        GEDUO_ASSERT(m_state == TERM || m_state == EXCEPT || m_state == INIT);
        StackAllocator::Dealloc(m_stack, m_stacksize);
    } else {
        GEDUO_ASSERT(!m_cb);
        GEDUO_ASSERT(m_state == EXEC);

        if (t_fiber == this) {
            SetThis(nullptr);
        }
    }
    GEDUO_LOG_DEBUG(g_logger) << "Fiber::~Fiber id = " << m_id
                              << " total = " << s_fiber_count;
}

/// 重置协程函数，并重置协程状态，要求协程状态为 INIT TERM 或 EXCEPT
void Fiber::reset(std::function<void()> cb) {
    GEDUO_ASSERT(m_stack);
    GEDUO_ASSERT(m_state == TERM || m_state == EXCEPT || m_state == INIT);
    m_cb = cb;
    if(getcontext(&m_ctx)) GEDUO_ASSERT2(false, "getcontext");

    m_ctx.uc_link = nullptr;
    m_ctx.uc_stack.ss_sp = m_stack;
    m_ctx.uc_stack.ss_size = m_stacksize;

    makecontext(&m_ctx, &Fiber::MainFunc, 0);
    m_state = INIT;
}

void Fiber::call() {
    SetThis(this);
    m_state = EXEC;
    if(swapcontext(&t_threadFiber->m_ctx, &m_ctx)) {
        GEDUO_ASSERT2(false, "swapcontext");
    }
}

void Fiber::back() {
    SetThis(t_threadFiber.get());
    if(swapcontext(&m_ctx, &t_threadFiber->m_ctx)) {
        GEDUO_ASSERT2(false, "swapcontext");
    }
}

void Fiber::swapIn() {
    SetThis(this);
    GEDUO_ASSERT(m_state != EXEC);
    m_state = EXEC;
    if(swapcontext(&Scheduler::GetMainFiber()->m_ctx, &m_ctx))
        GEDUO_ASSERT2(false, "swapcontext");
}

void Fiber::swapOut() {
    SetThis(Scheduler::GetMainFiber());
    if(swapcontext(&m_ctx, &Scheduler::GetMainFiber()->m_ctx))
        GEDUO_ASSERT2(false, "swapcontext");
}

void Fiber::SetThis(Fiber* f) {
    t_fiber = f;
}

Fiber::ptr Fiber::GetThis() {
    if(t_fiber) return t_fiber->shared_from_this();

    Fiber::ptr main_fiber(new Fiber);
    GEDUO_ASSERT(t_fiber == main_fiber.get());
    t_threadFiber = main_fiber;
    return t_fiber->shared_from_this();
}

void Fiber::YieldToReady() {
    Fiber::ptr cur = GetThis();
    GEDUO_ASSERT(cur->m_state == EXEC);
    cur->m_state = READY;
    cur->swapOut();
}

void Fiber::YieldToHold() {
    Fiber::ptr cur = GetThis();
    GEDUO_ASSERT(cur->m_state == EXEC);
    //cur->m_state = HOLD; //实际上是切换到后台执行，所以不算 HOLD 状态
    cur->swapOut();
}

uint64_t Fiber::TotalFibers() {
    return s_fiber_count;
}

void Fiber::MainFunc() {
    Fiber::ptr cur = GetThis();
    GEDUO_ASSERT(cur);
    try {
        cur->m_cb();
        cur->m_cb = nullptr;
        cur->m_state = TERM;
    } catch (std::exception& ex) {
        cur->m_state = EXCEPT;
        GEDUO_LOG_ERROR(g_logger) << "Fiber Except: " << ex.what()
            << " fiber_id = " << cur->getId()
            << std::endl
            << geduo::BacktraceToString();
    } catch(...) {
        cur->m_state = EXCEPT;
        GEDUO_LOG_ERROR(g_logger) << "Fiber Except: "
            << " fiber_id=" << cur->getId()
            << std::endl
            << geduo::BacktraceToString();
    }

    auto raw_ptr = cur.get();
    cur.reset();
    raw_ptr->swapOut();

    GEDUO_ASSERT2(false, "never reach fiber_id = " + std::to_string(raw_ptr->getId()));
}

void Fiber::CallerMainFunc() {
    Fiber::ptr cur = GetThis();
    GEDUO_ASSERT(cur);
    try {
        cur->m_cb();
        cur->m_cb = nullptr;
        cur->m_state = TERM;
    } catch (std::exception& ex) {
        cur->m_state = EXCEPT;
        GEDUO_LOG_ERROR(g_logger) << "Fiber Except: " << ex.what()
            << " fiber_id = " << cur->getId()
            << std::endl
            << geduo::BacktraceToString();
    } catch (...) {
        cur->m_state = EXCEPT;
        GEDUO_LOG_ERROR(g_logger) << "Fiber Except"
            << " fiber_id = " << cur->getId()
            << std::endl
            << geduo::BacktraceToString();
    }

    auto raw_ptr = cur.get();
    cur.reset();
    raw_ptr->back();
    GEDUO_ASSERT2(false, "never reach fiber_id = " + std::to_string(raw_ptr->getId()));
}

FiberSemaphore::FiberSemaphore(size_t initial_concurrency)
    : m_concurrency(initial_concurrency) {}

FiberSemaphore::~FiberSemaphore() {
    GEDUO_ASSERT(m_waiters.empty());
}

bool FiberSemaphore::tryWait() {
    GEDUO_ASSERT(Scheduler::GetThis());
    {
        MutexType::Lock lock(m_mutex);
        if (m_concurrency > 0u) {
            --m_concurrency;
            return true;
        }
        return false;
    }
}

void FiberSemaphore::wait()
{
    GEDUO_ASSERT(Scheduler::GetThis());
    {
        MutexType::Lock lock(m_mutex);
        if (m_concurrency > 0u) {
            --m_concurrency;
            return;
        }
        m_waiters.push_back(std::make_pair(Scheduler::GetThis(), Fiber::GetThis()));
    }
    Fiber::YieldToHold();
}

void FiberSemaphore::notify(){
    MutexType::Lock lock(m_mutex);
    if (!m_waiters.empty()) {
        auto next = m_waiters.front();
        m_waiters.pop_front();
        next.first->schedule(next.second);
    } else {
        ++m_concurrency;
    }
}
}

