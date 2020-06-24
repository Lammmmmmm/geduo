/*
 * @Author: choubin
 * @Date: 2020-06-14 22:53:25
 * @LastEditors: choubin
 * @LastEditTime: 2020-06-14 22:58:38
 * @Description: 不可拷贝对象封装
 */ 

#ifndef __GEDUO_NONCOPYABLE_H__
#define __GEDUO_NONCOPYABLE_H__

namespace geduo {

/// @brief 无拷贝、赋值函数的类
class Noncopyable {
public:
    Noncopyable() = default;
    virtual ~Noncopyable() = default; // 是否可为虚函数存疑

    Noncopyable(const Noncopyable&) = delete;
    Noncopyable& operator=(const Noncopyable&) = delete;
};
} // namespace geduo


#endif
