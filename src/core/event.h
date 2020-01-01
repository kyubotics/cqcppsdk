#pragma once

#include "./common.h"
#include "./target.h"
#include "./type.h"

namespace cq {
    struct Event {
        time_t time; // 酷Q触发事件的时间
        std::string type; // 事件类型
        std::string detail_type; // 事件详细类型
        std::string sub_type; // 事件子类型

        // 事件全名
        std::string name() const { return type + "." + detail_type + "." + sub_type; }
    };

    // 保留备用
    struct SysEvent : Event {};

    // 用户事件
    struct UserEvent : Event {
        struct Type {
            static constexpr char *MESSAGE = "message"; // 消息事件
            static constexpr char *NOTICE = "notice"; // 通知事件
            static constexpr char *REQUEST = "request"; // 请求事件
        };

        Target target; // 触发事件的主体, 例如某用户, 或某群的某用户, 可用于发送回复时指定目标

        enum Operation {
            IGNORE = 0,
            BLOCK = 1,
        };

        mutable Operation operation = IGNORE;
        // 阻止事件传递给下一个插件
        void block() const { operation = BLOCK; }
    };

    // 消息事件
    struct MessageEvent : UserEvent {
        struct DetailType {
            static constexpr char *PRIVATE = "private"; // 私聊消息
            static constexpr char *GROUP = "group"; // 群消息(包括新式讨论组)
            static constexpr char *DISCUSS = "discuss"; // 讨论组消息(仅旧式讨论组)
        };

        struct SubType {
            static constexpr char *DEFAULT = "default"; // 默认
            static constexpr char *FRIEND = "friend"; // 好友消息
            static constexpr char *GROUP = "group"; // 群临时会话
            static constexpr char *DISCUSS = "discuss"; // 讨论组临时会话
            static constexpr char *OTHER = "other"; // 陌生人消息
            static constexpr char *UNKNOWN = "unknown"; // 未知子类型
        };

        int32_t message_id; // 消息 Id
        std::string message; // 消息内容
        int32_t font; // 字体, 此属性已经没有实际意义
    };

    // 通知事件
    struct NoticeEvent : UserEvent {
        struct DetailType {
            static constexpr char *GROUP_UPLOAD = "group_upload"; // 群文件上传
            static constexpr char *GROUP_ADMIN = "group_admin"; // 群管理员变动
            static constexpr char *GROUP_MEMBER_DECREASE = "group_member_decrease"; // 群成员减少
            static constexpr char *GROUP_MEMBER_INCREASE = "group_member_increase"; // 群成员增加
            static constexpr char *GROUP_BAN = "group_ban"; // 群禁言
            static constexpr char *FRIEND_ADD = "friend_add"; // 好友已添加
        };

        struct SubType {
            static constexpr char *DEFAULT = "default"; // 默认
            static constexpr char *UNSET = "unset"; // 取消设置管理员
            static constexpr char *SET = "set"; // 设置管理员
            static constexpr char *LEAVE = "leave"; // 群成员退群
            static constexpr char *KICK = "kick"; // 群成员被踢
            static constexpr char *KICK_ME = "kick_me"; // 自己(登录号)被踢
            static constexpr char *APPROVE = "approve"; // 管理员已同意
            static constexpr char *INVITE = "invite"; // 管理员邀请
            static constexpr char *LIFT_BAN = "lift_ban"; // 解除禁言
            static constexpr char *BAN = "ban"; // 禁言
            static constexpr char *UNKNOWN = "unknown"; // 未知子类型
        };
    };

    // 请求事件
    struct RequestEvent : UserEvent {
        enum Operation {
            APPROVE = 1, // 同意
            REJECT = 2, // 拒绝
        };

        struct DetailType {
            static constexpr char *FRIEND = "friend"; // 好友请求
            static constexpr char *GROUP = "group"; // 群请求
        };

        struct SubType {
            static constexpr char *DEFAULT = "default"; // 默认
            static constexpr char *ADD = "add"; // 他人申请入群
            static constexpr char *INVITE = "invite"; // 自己(登录号)受邀入群
            static constexpr char *UNKNOWN = "unknown"; // 未知子类型
        };

        std::string comment; // 备注内容
        std::string flag; // 请求标识
    };

    struct UserIdMixin {
        int64_t user_id; // 用户 Id (QQ 号)
    };

    struct GroupIdMixin {
        int64_t group_id; // 群 Id (群号)
    };

    struct DiscussIdMixin {
        int64_t discuss_id; // 讨论组 Id
    };

    struct OperatorIdMixin {
        int64_t operator_id; // 操作者 Id (QQ 号)
    };

    // 私聊消息事件
    struct PrivateMessageEvent final : MessageEvent, UserIdMixin {
        PrivateMessageEvent() {
            type = Type::MESSAGE;
            detail_type = DetailType::PRIVATE;
        }
    };

    // 群消息事件
    struct GroupMessageEvent final : MessageEvent, UserIdMixin, GroupIdMixin {
        GroupMessageEvent() {
            type = Type::MESSAGE;
            detail_type = DetailType::GROUP;
        }

        Anonymous anonymous; // 匿名信息

        // 是否匿名消息
        bool is_anonymous() const { return !anonymous.name.empty(); }
    };

    // 讨论组消息事件
    struct DiscussMessageEvent final : MessageEvent, UserIdMixin, DiscussIdMixin {
        DiscussMessageEvent() {
            type = Type::MESSAGE;
            detail_type = DetailType::DISCUSS;
        }
    };

    // 群文件上传事件
    struct GroupUploadEvent final : NoticeEvent, UserIdMixin, GroupIdMixin {
        GroupUploadEvent() {
            type = Type::NOTICE;
            detail_type = DetailType::GROUP_UPLOAD;
        }

        File file; // 文件信息
    };

    // 群管理员变动事件
    struct GroupAdminEvent final : NoticeEvent, UserIdMixin, GroupIdMixin {
        GroupAdminEvent() {
            type = Type::NOTICE;
            detail_type = DetailType::GROUP_ADMIN;
        }
    };

    // 群成员减少事件
    struct GroupMemberDecreaseEvent final : NoticeEvent, UserIdMixin, GroupIdMixin, OperatorIdMixin {
        GroupMemberDecreaseEvent() {
            type = Type::NOTICE;
            detail_type = DetailType::GROUP_MEMBER_DECREASE;
        }
    };

    // 群成员增加事件
    struct GroupMemberIncreaseEvent final : NoticeEvent, UserIdMixin, GroupIdMixin, OperatorIdMixin {
        GroupMemberIncreaseEvent() {
            type = Type::NOTICE;
            detail_type = DetailType::GROUP_MEMBER_INCREASE;
        }
    };

    // 群禁言事件
    struct GroupBanEvent final : NoticeEvent, UserIdMixin, GroupIdMixin, OperatorIdMixin {
        GroupBanEvent() {
            type = Type::NOTICE;
            detail_type = DetailType::GROUP_BAN;
        }

        int64_t duration; // 禁言时长(秒)
    };

    // 好友已添加事件
    struct FriendAddEvent final : NoticeEvent, UserIdMixin {
        FriendAddEvent() {
            type = Type::NOTICE;
            detail_type = DetailType::FRIEND_ADD;
        }
    };

    // 好友请求事件
    struct FriendRequestEvent final : RequestEvent, UserIdMixin {
        FriendRequestEvent() {
            type = Type::REQUEST;
            detail_type = DetailType::FRIEND;
        }
    };

    // 群请求事件
    struct GroupRequestEvent final : RequestEvent, UserIdMixin, GroupIdMixin {
        GroupRequestEvent() {
            type = Type::REQUEST;
            detail_type = DetailType::GROUP;
        }
    };
} // namespace cq

namespace cq {
#define DEF_EVENT(EventName, EventType)                                          \
    extern std::vector<std::function<void(EventType)>> _##EventName##_callbacks; \
    inline void on_##EventName(std::function<void(EventType)> cb) { _##EventName##_callbacks.push_back(cb); }

    /*
    展开为:

    extern std::vector<std::function<void()>> _initialize_callbacks;
    inline void on_initialize(std::function<void()> cb) {
        _initialize_callbacks.push_back(cb);
    }

    extern std::vector<std::function<void(const PrivateMessageEvent &)>> _private_message_callbacks;
    inline void on_private_message(std::function<void(const PrivateMessageEvent &)> cb) {
        _private_message_callbacks.push_back(cb);
    }
    */

#include "./events.inc"

#undef DEF_EVENT
} // namespace cq
