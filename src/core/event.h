#pragma once

#include "./common.h"
#include "./target.h"

namespace cq {
    struct Event {
        time_t time; // 酷Q触发事件的时间
        std::string type; // 事件类型
        std::string detail_type; // 事件详细类型
        std::string sub_type; // 事件子类型

        std::string name() const { return type + "." + detail_type + "." + sub_type; }
    };

    // 保留备用
    struct SysEvent : Event {};

    struct UserEvent : Event {
        struct Type {
            static constexpr char *MESSAGE = "message";
            static constexpr char *NOTICE = "notice";
            static constexpr char *REQUEST = "request";
        };

        Target target;

        enum Operation {
            IGNORE = 0,
            BLOCK = 1,
        };

        mutable Operation operation = IGNORE;
        void block() const { operation = BLOCK; }
    };

    struct MessageEvent : UserEvent {
        struct DetailType {
            static constexpr char *PRIVATE = "private";
            static constexpr char *GROUP = "group";
            static constexpr char *DISCUSS = "discuss";
        };

        struct SubType {
            static constexpr char *DEFAULT = "default";
            static constexpr char *FRIEND = "friend";
            static constexpr char *GROUP = "group";
            static constexpr char *DISCUSS = "discuss";
            static constexpr char *OTHER = "other";
            static constexpr char *UNKNOWN = "unknown";
        };

        int32_t message_id;
        std::string raw_message;
        std::string message; // TODO
        int32_t font;
    };

    struct NoticeEvent : UserEvent {
        struct DetailType {
            static constexpr char *GROUP_UPLOAD = "group_upload";
            static constexpr char *GROUP_ADMIN = "group_admin";
            static constexpr char *GROUP_MEMBER_DECREASE = "group_member_decrease";
            static constexpr char *GROUP_MEMBER_INCREASE = "group_member_increase";
            static constexpr char *GROUP_BAN = "group_ban";
            static constexpr char *FRIEND_ADD = "friend_add";
        };

        struct SubType {
            static constexpr char *DEFAULT = "default";
            static constexpr char *UNSET = "unset";
            static constexpr char *SET = "set";
            static constexpr char *LEAVE = "leave";
            static constexpr char *KICK = "kick";
            static constexpr char *KICK_ME = "kick_me";
            static constexpr char *APPROVE = "approve";
            static constexpr char *INVITE = "invite";
            static constexpr char *LIFT_BAN = "lift_ban";
            static constexpr char *BAN = "ban";
            static constexpr char *UNKNOWN = "unknown";
        };
    };

    struct RequestEvent : UserEvent {
        enum Operation {
            APPROVE = 1,
            REJECT = 2,
        };

        struct DetailType {
            static constexpr char *FRIEND = "friend";
            static constexpr char *GROUP = "group";
        };

        struct SubType {
            static constexpr char *DEFAULT = "default";
            static constexpr char *ADD = "add";
            static constexpr char *INVITE = "invite";
            static constexpr char *UNKNOWN = "unknown";
        };

        std::string comment;
        std::string flag;
    };

    struct UserIdMixin {
        int64_t user_id;
    };

    struct GroupIdMixin {
        int64_t group_id;
    };

    struct DiscussIdMixin {
        int64_t discuss_id;
    };

    struct OperatorIdMixin {
        int64_t operator_id;
    };

    struct PrivateMessageEvent final : MessageEvent, UserIdMixin {
        PrivateMessageEvent() {
            type = Type::MESSAGE;
            detail_type = DetailType::PRIVATE;
        }
    };

    struct GroupMessageEvent final : MessageEvent, UserIdMixin, GroupIdMixin {
        GroupMessageEvent() {
            type = Type::MESSAGE;
            detail_type = DetailType::GROUP;
        }

        // Anonymous anonymous;  // TODO

        // bool is_anonymous() const { return !anonymous.name.empty(); }
    };

    struct DiscussMessageEvent final : MessageEvent, UserIdMixin, DiscussIdMixin {
        DiscussMessageEvent() {
            type = Type::MESSAGE;
            detail_type = DetailType::DISCUSS;
        }
    };

    struct GroupUploadEvent final : NoticeEvent, UserIdMixin, GroupIdMixin {
        GroupUploadEvent() {
            type = Type::NOTICE;
            detail_type = DetailType::GROUP_UPLOAD;
        }

        // File file;  // TODO
    };

    struct GroupAdminEvent final : NoticeEvent, UserIdMixin, GroupIdMixin {
        GroupAdminEvent() {
            type = Type::NOTICE;
            detail_type = DetailType::GROUP_ADMIN;
        }
    };

    struct GroupMemberDecreaseEvent final : NoticeEvent, UserIdMixin, GroupIdMixin, OperatorIdMixin {
        GroupMemberDecreaseEvent() {
            type = Type::NOTICE;
            detail_type = DetailType::GROUP_MEMBER_DECREASE;
        }
    };

    struct GroupMemberIncreaseEvent final : NoticeEvent, UserIdMixin, GroupIdMixin, OperatorIdMixin {
        GroupMemberIncreaseEvent() {
            type = Type::NOTICE;
            detail_type = DetailType::GROUP_MEMBER_INCREASE;
        }
    };

    struct GroupBanEvent final : NoticeEvent, UserIdMixin, GroupIdMixin, OperatorIdMixin {
        GroupBanEvent() {
            type = Type::NOTICE;
            detail_type = DetailType::GROUP_BAN;
        }

        int64_t duration;
    };

    struct FriendAddEvent final : NoticeEvent, UserIdMixin {
        FriendAddEvent() {
            type = Type::NOTICE;
            detail_type = DetailType::FRIEND_ADD;
        }
    };

    struct FriendRequestEvent final : RequestEvent, UserIdMixin {
        FriendRequestEvent() {
            type = Type::REQUEST;
            detail_type = DetailType::FRIEND;
        }
    };

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
