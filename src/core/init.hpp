#pragma once

#include "common.hpp"

namespace cq {
    // Auth Code, 在酷Q的 Initialize 事件中获得
    inline int32_t &_ac() {
        static int32_t ac;
        return ac;
    }

    // 初始化函数, 在酷Q的 Initialize 事件中调用
    inline auto &_init_impl() {
        static void (*init_impl)();
        return init_impl;
    }

    inline void _init() {
        if (_init_impl()) {
            _init_impl()();
        }
    }
} // namespace cq

#define CQ_INIT                                  \
    void __cq_init_impl();                       \
    bool __cq_set_init_impl() {                  \
        cq::_init_impl() = __cq_init_impl;       \
        return true;                             \
    }                                            \
    bool __cq_init_dummy = __cq_set_init_impl(); \
    void __cq_init_impl()
