/*
 * @Author: choubin
 * @Date: 2020-06-14 22:38:28
 * @LastEditors: choubin
 * @LastEditTime: 2020-06-14 22:51:07
 * @Description: 单例模式封装
 */ 

#ifndef __GEDUO_SINGLETON_H__
#define __GEDUO_SINGLETON_H__

#include <memory>

namespace geduo {

/// @brief 单例模式裸指针封装类，X 为了创造读个实例对应的Tag，N为一个 Tag 创造多个实例索引
template<class T, class X = void, int N = 0>
class Singleton {
public:
    /// @brief 返回单例裸指针
    static T* GetInstance() {
        static T v;
        return &v;
    }
};

/// @brief 单例模式智能指针封装类
template<class T, class X = void, int N = 0>
class SingletonPtr {
    /// @brief 返回单例智能指针
    static std::shared_ptr<T> GetInstance() {
        static std::shared_ptr<T> v(new T);
        return v;
    }
};
} // namespace geduo


#endif