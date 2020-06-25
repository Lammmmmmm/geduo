/*
 * @file : fiber.h
 * @brief: 协程封装
 * @author: Choupin
 * @email: 593214609@qq.com
 * @date: Do not edit
 * @LastEditTime: 2020-06-26 01:01:02
 * @LastEditors: Choubin
 */

#ifndef __GEDUO_FIBER_H__
#define __GEDUO_FIBER_H__

#include <ucontext.h>

#include <functional>
#include <memory>

#include "mutex.h"

namespace geduo {

class Scheduler;

/// @brief 协程类
class Fiber : public std::enable_shared_from_this<Fiber> {
friend class scheduler;
public:
    typedef std::shared_ptr<Fiber> ptr;

    /// @brief 协程状态
    enum State { INIT, HOLD, EXEC, TERM, READY, EXCEPT };

private:
    Fiber();

public:
    Fiber(std::function<void()> cb, size_t stacksize = 0,
        bool use_caller = false);
    ~Fiber();
    /// @brief 重置协程执行函数，并这是状态为 INIT
    void reset(std::function<void()> cb);
    /// @bried 切换到当前协程执行
    void swapIn();
    /// @brief 将当前协程切换到后台执行
    void swapOut();
    /// @brief 将当前协程切换到执行状态
    void call();
    /// @brief 将当前协程切换到后台
    void back();
    /// @brief 返回协程 id
    uint64_t getId() const { return m_id; }
    /// @brief 返回协程状态
    State getState() const { return m_state; }
    /// @brief 设置当前线程的运行协程
    static void SetThis(Fiber* f);
    /// @brief 返回当前所在的协程
    static Fiber::ptr GetThis();
    /// @brief 将当前协程切换到后台，并设置为 READY 状态
    static void YieldToReady();
    /// @brief 将当前协程切换到后台，并设置为 HOLD 状态
    static void YieldToHold();
    /// @brief 返回当前协程的总数量
    static uint64_t TotalFibers();
    /// @brief 协程执行函数，执行完返回线程主协程
    static void MainFunc();
    /// @brief 协程执行函数，执行完返回线程调度协程
    static void CallerMainFunc();
    /// @brief 获取当前协程的 id
    static uint64_t GetFiberId();

private:
    uint64_t m_id = 0; /// 协程 id
    uint32_t m_stacksize = 0; /// 协程运行栈大小
    State m_state = INIT; /// 协程状态
    ucontext_t m_ctx; /// 协程上下文
    void* m_stack = nullptr; /// 协程运行栈指针
    std::function<void()> m_cb; /// 协程运行函数
};

class FiberSemaphore : Noncopyable {
public:
    typedef Spinlock MutexType;

    FiberSemaphore(size_t initial_concurrency = 0);
    ~FiberSemaphore();

    bool tryWait();
    void wait();
    void notify();

    size_t getConcurrency() const { return m_concurrency; }
    void reset() { m_concurrency = 0; }

private:
    MutexType m_mutex;
    std::list<std::pair<Scheduler*, Fiber::ptr>> m_waiters;
    size_t m_concurrency;
};

} // namespace geduo

#endif
