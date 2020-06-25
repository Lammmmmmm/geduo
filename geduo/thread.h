/*
 * @Author: choubin
 * @Date: 2020-06-14 22:52:46
 * @LastEditors: Choubin
 * @LastEditTime: 2020-06-26 01:02:22
 * @Description: file content
 */

#ifndef __GEDUO_THREAD_H__
#define __GEDUO_THREAD_H__

#include "mutex.h"

namespace geduo {

/// @brief 线程类，持有一个线程
class Thread : Noncopyable {
public:
    typedef std::shared_ptr<Thread> ptr;

    Thread(std::function<void()> cb, const std::string& name);
    ~Thread();

    /// @brief 返回线程ID
    pid_t getId() const { return m_id; }

    /// @brief 返回线程名称
    const std::string& getName() const { return m_name; }

    /// @brief 等待线程执行完成
    void join();

    /// @brief 获取当前线程指针
    static Thread* GetThis();

    /// @brief 获取当前线程名称
    static const std::string& GetName();

    /// @brief 设置当前线程名称
    static void SetName(const std::string& name);

private:
    /// @brief 执行线程函数
    static void* run(void* arg);

private:
    pid_t m_id = -1; /// 线程 ID
    pthread_t m_thread = 0; /// 线程结构
    std::function<void()> m_cb; /// 线程执行函数
    std::string m_name; /// 线程名称
    Semaphore m_semaphore; /// 信号量
};

} // namespace geduo

#endif