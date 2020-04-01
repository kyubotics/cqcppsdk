#pragma once

#include "common.hpp"

#include "event.hpp"

namespace cq {
#define DEF_EVENT(EventName, EventType)                                           \
    inline auto &_##EventName##_callbacks() {                                     \
        static std::vector<std::function<void(EventType)>> EventName##_callbacks; \
        return EventName##_callbacks;                                             \
    }                                                                             \
    inline void on_##EventName(const std::function<void(EventType)> &cb) {        \
        _##EventName##_callbacks().push_back(cb);                                 \
    }

    /*
    展开为:

    inline auto &_initialize_callbacks() {
        static std::vector<std::function<void()>> initialize_callbacks;
        return initialize_callbacks;
    }

    inline void on_initialize(const std::function<void()> &cb) {
        _initialize_callbacks().push_back(cb);
    }

    inline auto &_private_message_callbacks() {
        static std::vector<std::function<void(EventType)>> private_message_callbacks;
        return private_message_callbacks;
    }

    inline void on_private_message(const std::function<void(const PrivateMessageEvent &)> &cb) {
        _private_message_callbacks().push_back(cb);
    }

    ...
    */

    DEF_EVENT(initialize, ) // 初始化
    DEF_EVENT(enable, ) // 应用启用
    DEF_EVENT(disable, ) // 应用停用
    DEF_EVENT(coolq_start, ) // 酷Q启动
    DEF_EVENT(coolq_exit, ) // 酷Q退出

    DEF_EVENT(private_message, const PrivateMessageEvent &) // 私聊消息事件
    DEF_EVENT(group_message, const GroupMessageEvent &) // 群消息事件
    DEF_EVENT(discuss_message, const DiscussMessageEvent &) // 讨论组消息事件

    DEF_EVENT(message, const MessageEvent &) // 所有消息事件

    DEF_EVENT(group_upload, const GroupUploadEvent &) // 群文件上传事件
    DEF_EVENT(group_admin, const GroupAdminEvent &) // 群管理员变动事件
    DEF_EVENT(group_member_decrease, const GroupMemberDecreaseEvent &) // 群成员减少事件
    DEF_EVENT(group_member_increase, const GroupMemberIncreaseEvent &) // 群成员增加事件
    DEF_EVENT(group_ban, const GroupBanEvent &) // 群禁言事件
    DEF_EVENT(friend_add, const FriendAddEvent &) // 好友添加事件

    DEF_EVENT(notice, const NoticeEvent &) // 所有通知事件

    DEF_EVENT(friend_request, const FriendRequestEvent &) // 好友请求事件
    DEF_EVENT(group_request, const GroupRequestEvent &) // 群请求事件

    DEF_EVENT(request, const RequestEvent &) // 所有请求事件

#undef DEF_EVENT
} // namespace cq
