#pragma once

#include "event_export_def.hpp"

#define CQ_MENU(FuncName)               \
    static void __cq_menu_##FuncName(); \
    _CQ_EVENT(int32_t, FuncName, 0)() { \
        __cq_menu_##FuncName();         \
        return 0;                       \
    }                                   \
    static void __cq_menu_##FuncName()
