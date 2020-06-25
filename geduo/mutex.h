/*
 * @Author: choubin
 * @Date: 2020-06-14 22:52:57
 * @LastEditors: Choubin
 * @LastEditTime: 2020-06-25 20:42:13
 * @Description: file content
 */ 

#ifndef __GEDUO_MUTEX_H__
#define __GEDUO_MUTEX_H__

#include <thread>
#include <functional>
#include <memory>
#include <pthread.h>
#include <semaphore.h>
#include <stdint.h>
#include <atomic>
#include <list>

#include "noncopyable.h"

namespace geduo {

/// @brief 信号量
class Semaphore : Noncopyable {
public:
    Semaphore(uint32_t count = 0) {
        if(sem_init(&m_semaphore, 0, count)) {
            throw std::logic_error("sem_init error");
        }
    }
    ~Semaphore() {
        if(sem_destroy(&m_semaphore)) {
            throw std::logic_error("sem_destory error");
        }
    }

    /// @brief 获取信号量
    void wait() {
        if(sem_wait(&m_semaphore)) {
            throw std::logic_error("sem_wait error");
        }
    }

    /// @brief 释放信号量
    void notify() {
        if(sem_post(&m_semaphore)) {
            throw std::logic_error("sem_post error");
        }
    }
private:
    sem_t m_semaphore;    
};

/// @brief 局部锁模板类
template<class T>
struct ScopeLockImpl {
public:
    ScopeLockImpl(T& mutex) : m_mutex(mutex) {
        m_mutex.lock();
        m_locked = true;
    }
    ~ScopeLockImpl() {
        unlock();
    }

    /// @brief 加锁
    void lock() {
        if (!m_locked) {
            m_mutex.lock();
            m_locked = true;
        }
    }

    /// @brief 解锁
    void unlock() {
        if(m_locked) {
            m_mutex.unlock();
            m_locked = false;
        }
    }
private:
    T& m_mutex;
    bool m_locked;
};

/// @brief 局部读锁
template<class T>
struct ReadScopedLockImpl {
public:
    ReadScopedLockImpl(T& mutex): m_mutex(mutex) {
        m_mutex.rdlock();
        m_locked = true;
    }
    ~ReadScopedLockImpl() {
        unlock();
    }

    void lock() {
        if(!m_locked) {
            m_mutex.rdlock();
            m_locked = true;
        }
    }

    void unlock() {
        if(m_locked) {
            m_mutex.unlock();
            m_locked = false;
        }
    }
private:
    T& m_mutex;
    bool m_locked;
};

/// @brief 局部写锁
template<class T>
struct WriteScopedLockImpl {
public:
    WriteScopedLockImpl(T& mutex)
        :m_mutex(mutex) {
        m_mutex.wrlock();
        m_locked = true;
    }

    ~WriteScopedLockImpl() {
        unlock();
    }

    void lock() {
        if(!m_locked) {
            m_mutex.wrlock();
            m_locked = true;
        }
    }

    void unlock() {
        if(m_locked) {
            m_mutex.unlock();
            m_locked = false;
        }
    }
private:
    T& m_mutex;
    bool m_locked;
};

/// @brief 互斥量
class Mutex: Noncopyable {
public:
    /// 局部所
    typedef ScopeLockImpl<Mutex> Lock;

    Mutex() {
        pthread_mutex_init(&m_mutex, nullptr);
    }

    ~Mutex() {
        pthread_mutex_destroy(&m_mutex);
    }

    void lock() {
        pthread_mutex_lock(&m_mutex);
    }

    void unlock() {
        pthread_mutex_unlock(&m_mutex);
    }
private:
    pthread_mutex_t m_mutex;
};

/// @brief 空锁，用于调试
class NullMutex : Noncopyable {
    typedef ScopeLockImpl<NullMutex> Lock;
    NullMutex() {}
    ~NullMutex() {}
    void lock() {}
    void unlock() {}
};

/// @brief 读写互斥量
class RWMutex : Noncopyable {
public:
    typedef ReadScopedLockImpl<RWMutex> ReadLock;
    typedef WriteScopedLockImpl<RWMutex> WriteLock;

    RWMutex() {
        pthread_rwlock_init(&m_mutex, nullptr);
    }

    ~RWMutex() {
        pthread_rwlock_destroy(&m_mutex);
    }
    /// @brief 上读锁
    void rdlock() {
        pthread_rwlock_rdlock(&m_mutex);
    }
    /// @brief 上写锁
    void wrlock() {
        pthread_rwlock_wrlock(&m_mutex);
    }
    /// @brief 解锁
    void unlock() {
        pthread_rwlock_unlock(&m_mutex);
    }
private:
    pthread_rwlock_t m_mutex;
};

/// @brief 空读写锁(用于调试)
class NullRWMutex : Noncopyable {
public:
    typedef ReadScopedLockImpl<NullMutex> ReadLock;
    typedef WriteScopedLockImpl<NullMutex> WriteLock;
    NullRWMutex() {}
    ~NullRWMutex() {}
    void rdlock() {}
    void wrlock() {}
    void unlock() {}
};

/// @brief 自旋锁
class Spinlock : Noncopyable {
public:
    typedef ScopeLockImpl<Spinlock> Lock;

    Spinlock() {
        pthread_spin_init(&m_mutex, 0);
    }

    ~Spinlock() {
        pthread_spin_destroy(&m_mutex);
    }

    /// @brief 上锁
    void lock() {
        pthread_spin_lock(&m_mutex);
    }

    /// @brief 解锁
    void unlock() {
        pthread_spin_unlock(&m_mutex);
    }
private:
    pthread_spinlock_t m_mutex;
};

/// @brief 原子锁
class CASLock : Noncopyable {
public:
    typedef ScopeLockImpl<CASLock> Lock;

    CASLock() {
        m_mutex.clear();
    }

    ~CASLock() {}

    /// @brief 上锁
    void lock() {
        while (std::atomic_flag_test_and_set_explicit(&m_mutex, std::memory_order_acquire));
    }

    /// @brief 解锁
    void unlock() {
        std::atomic_flag_clear_explicit(&m_mutex, std::memory_order_release);
    }
private:
    /// 原子状态
    volatile std::atomic_flag m_mutex;
};

} // namespace geduo


#endif