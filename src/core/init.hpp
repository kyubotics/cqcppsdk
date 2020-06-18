#pragma once

#include "common.hpp"

namespace cq {
    // Auth Code, 在酷Q的 Initialize 事件中获得
    inline int32_t &_ac() {
        static int32_t ac;
        return ac;
    }

    // 初始化函数, 在酷Q的 Initialize 事件中调用
    inline void _init() {
        extern void __init_impl();
        __init_impl();
    }
} // namespace cq

#define CQ_INIT             \
    namespace cq {          \
        void __init_impl(); \
    }                       \
    void cq::__init_impl()
