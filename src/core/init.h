#pragma once

#include "./common.h"

namespace cq {
    extern int32_t __ac; // Auth Code, 在酷Q的 Initialize 事件中获得
    extern void (*__init_impl)(); // 初始化函数, 在酷Q的 Initialize 事件中调用

    inline void __init() {
        if (__init_impl) {
            __init_impl();
        }
    }
} // namespace cq

#define CQ_INIT                                  \
    void __cq_init_impl();                       \
    bool __cq_set_init_impl() {                  \
        cq::__init_impl = __cq_init_impl;        \
        return true;                             \
    }                                            \
    bool __cq_init_dummy = __cq_set_init_impl(); \
    void __cq_init_impl()
