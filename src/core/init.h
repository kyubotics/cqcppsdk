#pragma once

#include "./common.h"

namespace cq {
    extern int32_t _ac; // Auth Code, 在酷Q的 Initialize 事件中获得
    extern void (*_init_impl)(); // 初始化函数, 在酷Q的 Initialize 事件中调用

    inline void _init() {
        if (_init_impl) {
            _init_impl();
        }
    }
} // namespace cq

#define CQ_INIT                                  \
    void __cq_init_impl();                       \
    bool __cq_set_init_impl() {                  \
        cq::_init_impl = __cq_init_impl;         \
        return true;                             \
    }                                            \
    bool __cq_init_dummy = __cq_set_init_impl(); \
    void __cq_init_impl()
