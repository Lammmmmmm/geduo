/*
 * @Author: choubin
 * @Date: 2020-06-14 22:35:40
 * @LastEditors: Choubin
 * @LastEditTime: 2020-06-25 18:58:08
 * @Description: 常用宏的封装
 */ 

#ifndef __GEDUO_MACRO_H__
#define __GEDUO_MACRO_H__

#include <string.h>
#include <assert.h>

#include "util.h"
#include "log.h"

#if defined __GNUC__ || defined __llvm__
/// LIKCLY 宏的封装, 告诉编译器优化,条件大概率成立
#   define GEDUO_LIKELY(x) __builtin_expect(!!(x), 1)
/// LIKCLY 宏的封装, 告诉编译器优化,条件大概率不成立
#   define GEDUO_UNLIKELY(x) __builtin_expect(!!(x), 0)
#else
#   define GEDUO_LIKELY(x) (x)
#   define GEDUO_UNLIKELY(x) (x)
#endif

/// 断言宏封装
#define GEDUO_ASSERT(x)                                                                \
    if (GEDUO_UNLIKELY(!(x))) {                                                        \
        GEDUO_LOG_ERROR(GEDUO_LOG_ROOT()) << "ASSERTION: " #x                          \
                                          << "\nbacktrace:\n"                          \
                                          << geduo::BacktraceToString(100, 2, "    "); \
        assert(x);                                                                     \
    }

/// 断言宏封装
#define GEDUO_ASSERT2(x, w)                                                            \
    if (GEDUO_UNLIKELY(!(x))) {                                                        \
        GEDUO_LOG_ERROR(GEDUO_LOG_ROOT()) << "ASSERTION: " #x                          \
                                          << "\n"                                      \
                                          << w                                         \
                                          << "\nbacktrace:\n"                          \
                                          << geduo::BacktraceToString(100, 2, "    "); \
        assert(x);                                                                     \
    }

#endif