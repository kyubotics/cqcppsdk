#pragma once

#include "./common.h"

namespace cq {
    extern int32_t __ac; // Auth Code, 在酷Q的 Initialize 事件中获得
    extern void (*__init)(); // 初始化函数, 在酷Q的 Initialize 事件中调用
} // namespace cq

#define CQ_INIT                                      \
    void __cq_init();                                \
    bool __cq_set_init_function() {                  \
        cq::__init = __cq_init;                      \
        return true;                                 \
    }                                                \
    bool __cq_init_dummy = __cq_set_init_function(); \
    void __cq_init()
