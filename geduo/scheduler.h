/*
 * @Author: Choubin
 * @Date: 2020-06-25 19:00:20
 * @LastEditors: Choubin
 * @LastEditTime: 2020-06-26 00:50:38
 * @FilePath: /geduo/geduo/scheduler.h
 * @Description:  协程调度器封装
 */ 

#ifndef __GEDUO_SCHEDULER_H__
#define __GEDUO_SCHEDULER_H__

#include <memory>
#include <vector>
#include <list>
#include <iostream>

#include "fiber.h"
#include "thread.h"

namespace geduo {

class Scheduler {
public:
    typedef std::shared_ptr<Scheduler> ptr;
    typedef Mutex MutexType;

    Scheduler(size_t threads = 1, bool use_caller = true, const std::string& name = "");
    virtual ~Scheduler();

    /// @brief 返回协程调度器名称
    const std::string& getName() const { return m_name; }

    /// @brief 返回当前协程调度器
    static Scheduler* GetThis();

    /// @brief 返回当前协程调度器的调度协程
    static Fiber* GetMainFiber();

    /// @brief 启动协程调度器
    void start();

    /// @brief 停止协程调度器
    void stop();

    /// @breif 调度协程
    template <typename FiberOrCb>
    void schedule(FiberOrCb fc, int thread = -1) {
        bool need_tickle = false;
        {
            MutexType::Lock lock(m_mutex);
            need_tickle = scheduleNoLock(fc, thread);
        }
        if (need_tickle) tickle();
    }

    /// @brief 批量调度协程, 输入为协程数组的迭代器表范围
    template <typename InputIterator>
    void schedule(InputIterator begin, InputIterator end) {
        bool need_tickle = false;
        {
            MutexType::Lock lock(m_mutex);
            while(begin != end) {
                need_tickle = scheduleNoLock(&*begin(), -1) || need_tickle;
                ++begin;
            }
        }
        if(need_tickle) tickle();
    }

    void switchTo(int thread = -1);
    std::ostream& dump(std::ostream& os);
protected:
    /// @brief 通知协程调度器有任务
    virtual void tickle();

    /// @brief 协程调度函数
    void run();

    /// @brief 返回是否可以停止
    virtual bool stopping();

    /// @brief 协程无任务可调度时执行 idle 协程
    virtual void idle();

    /// @brief 设置当前的协程调度器
    void setThis();

    /// @brief 是否有空闲线程
    bool hasIdleThreads() { return m_idleThreadCount > 0; }
private:
    /// @brief 协程调度启动
    template <typename FiberOrCb>
    bool scheduleNoLock(FiberOrCb fc, int thread) {
        bool need_tickle = m_fibers.empty(); // 只有待执行协程从无到有才需要通知
        FiberAndThread ft(fc, thread);
        if (ft.fiber || ft.cb)
            m_fibers.push_back(ft);
        return need_tickle;
    }
private:
    /// @brief 协程、函数、线程组
    struct FiberAndThread {
        Fiber::ptr fiber;
        std::function<void()> cb;
        int thread;

        FiberAndThread(Fiber::ptr f, int thr)
            :fiber(f), thread(thr){}

        FiberAndThread(Fiber::ptr* f, int thr)
            :thread(thr) {
            fiber.swap(*f);
        }

        FiberAndThread(std::function<void()> f, int thr)
            :cb(f), thread(thr) {}

        FiberAndThread(std::function<void()>* f, int thr)
            :thread(thr) {
                cb.swap(*f);
            }
        
        FiberAndThread()
            :thread(-1) {}
        
        void reset() {
            fiber = nullptr;
            cb = nullptr;
            thread = -1;
        }
    };
private:
    MutexType m_mutex;
    /// 线程池
    std::vector<Thread::ptr> m_threads;
    /// 待执行的协程队列
    std::list<FiberAndThread> m_fibers;
    /// 调度协程，use_caller 为 true 时有效
    Fiber::ptr m_rootFiber;
    /// 协程调度器名称
    std::string m_name;
protected:
    /// 协程下，线程 ID 数组
    std::vector<int> m_threadIds;
    /// 线程数量
    size_t m_threadCount = 0;
    /// 工作线程数量
    std::atomic<size_t> m_activeThreadCount = {0};
    /// 空闲线程数量
    std::atomic<size_t> m_idleThreadCount = {0};
    /// 是否正在停止
    bool m_stopping = true;
    /// 是否自动停止
    bool m_autoStop = false;
    /// 主线程 id(usr_caller)
    int m_rootThread = 0;
};

class SchedulerSwitcher : public Noncopyable {
public:
    SchedulerSwitcher(Scheduler* target = nullptr);
    ~SchedulerSwitcher();

private:
    Scheduler* m_caller;
};

} // namespace geduo


#endif