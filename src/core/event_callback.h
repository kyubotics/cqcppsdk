#pragma once

#include "./common.h"
#include "./event.h"

namespace cq {
#define DEF_EVENT(EventName, EventType)                                          \
    extern std::vector<std::function<void(EventType)>> _##EventName##_callbacks; \
    inline void on_##EventName(const std::function<void(EventType)> &cb) {       \
        _##EventName##_callbacks.push_back(cb);                                  \
    }

    /*
    展开为:

    extern std::vector<std::function<void()>> _initialize_callbacks;
    inline void on_initialize(const std::function<void()> &cb) {
        _initialize_callbacks.push_back(cb);
    }

    extern std::vector<std::function<void(const PrivateMessageEvent &)>> _private_message_callbacks;
    inline void on_private_message(const std::function<void(const PrivateMessageEvent &)> &cb) {
        _private_message_callbacks.push_back(cb);
    }
    */

#include "./events.inc"

#undef DEF_EVENT
} // namespace cq
