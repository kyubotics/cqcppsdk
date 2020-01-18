#pragma once

#include "./common.h"
#include "./target.h"
#include "./type.h"

namespace cq {
    struct Event {
        time_t time; // 酷Q触发事件的时间

        Event() : time(std::time(nullptr)) {
        }

        virtual ~Event() = default;
    };

    // 保留备用
    struct SysEvent : Event {
        using Event::Event;
        virtual ~SysEvent() = default;
    };

    // 用户事件
    struct UserEvent : Event {
        enum class Type {
            MESSAGE, // 消息事件
            NOTICE, // 通知事件
            REQUEST, // 请求事件
        };

        Type type; // 事件类型
        Target target; // 触发事件的主体, 例如某用户, 或某群的某用户, 可用于发送回复时指定目标

        enum class _ReturnCode {
            IGNORE = 0,
            BLOCK = 1,
        };

        mutable _ReturnCode operation = _ReturnCode::IGNORE;

        // 阻止事件传递给下一个应用
        void block() const {
            operation = _ReturnCode::BLOCK;
        }

        // 事件是否已被阻止传递给下一个应用
        bool blocked() const {
            return operation == _ReturnCode::BLOCK;
        }

        // 默认事件子类型枚举, 用于酷Q没有明确区分子类型的事件类, 以提供一致的接口
        enum class SubType {
            DEFAULT = 1,
        };

        UserEvent(Type type, Target &&target) : type(type), target(std::move(target)) {
        }

        virtual ~UserEvent() = default;
    };

    // 消息事件
    struct MessageEvent : UserEvent {
        enum class DetailType {
            PRIVATE, // 私聊消息
            GROUP, // 群消息(包括新式讨论组)
            DISCUSS, // 讨论组消息(仅旧式讨论组)
        };

        DetailType detail_type; // 事件详细类型
        int64_t message_id; // 消息 Id
        std::string message; // 消息内容
        int32_t font; // 字体, 此属性已经没有实际意义

        MessageEvent(Target &&target, DetailType detail_type, int64_t message_id, std::string &&message, int32_t font)
            : UserEvent(Type::MESSAGE, std::move(target)),
              detail_type(detail_type),
              message_id(message_id),
              message(std::move(message)),
              font(font) {
        }

        virtual ~MessageEvent() = default;
    };

    // 通知事件
    struct NoticeEvent : UserEvent {
        enum class DetailType {
            GROUP_UPLOAD, // 群文件上传
            GROUP_ADMIN, // 群管理员变动
            GROUP_MEMBER_DECREASE, // 群成员减少
            GROUP_MEMBER_INCREASE, // 群成员增加
            GROUP_BAN, // 群禁言
            FRIEND_ADD, // 好友添加
        };

        DetailType detail_type; // 事件详细类型

        NoticeEvent(Target &&target, DetailType detail_type)
            : UserEvent(Type::NOTICE, std::move(target)), detail_type(detail_type) {
        }

        virtual ~NoticeEvent() = default;
    };

    // 请求事件
    struct RequestEvent : UserEvent {
        enum class Operation {
            APPROVE = 1, // 同意
            REJECT = 2, // 拒绝
        };

        enum class DetailType {
            FRIEND, // 好友请求
            GROUP, // 群请求
        };

        // 请求标识包装类
        struct Flag {
            std::string raw;

            operator std::string() const {
                return raw;
            }
        };

        DetailType detail_type; // 事件详细类型
        std::string comment; // 备注内容
        Flag flag; // 请求标识

        RequestEvent(Target &&target, DetailType detail_type, std::string &&comment, Flag &&flag)
            : UserEvent(Type::REQUEST, std::move(target)),
              detail_type(detail_type),
              comment(std::move(comment)),
              flag(std::move(flag)) {
        }

        virtual ~RequestEvent() = default;
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
        enum class SubType {
            FRIEND = 11, // 好友消息
            GROUP = 2, // 群临时会话
            DISCUSS = 3, // 讨论组临时会话
            OTHER = 1, // 陌生人消息
        };

        SubType sub_type; // 事件子类型

        PrivateMessageEvent(int64_t message_id, std::string &&message, int32_t font, int64_t user_id, SubType sub_type)
            : MessageEvent(Target(user_id), DetailType::PRIVATE, message_id, std::move(message), font),
              UserIdMixin{user_id},
              sub_type(sub_type) {
        }
    };

    // 群消息事件
    struct GroupMessageEvent final : MessageEvent, UserIdMixin, GroupIdMixin {
        SubType sub_type = SubType::DEFAULT; // 默认事件子类型
        Anonymous anonymous; // 匿名信息

        // 是否匿名消息
        bool is_anonymous() const {
            return !anonymous.name.empty();
        }

        GroupMessageEvent(int64_t message_id, std::string &&message, int32_t font, int64_t user_id, int64_t group_id,
                          Anonymous &&anonymous)
            : MessageEvent(Target(user_id, group_id, Target::GROUP), DetailType::GROUP, message_id, std::move(message),
                           font),
              UserIdMixin{user_id},
              GroupIdMixin{group_id},
              anonymous(std::move(anonymous)) {
        }
    };

    // 讨论组消息事件
    struct DiscussMessageEvent final : MessageEvent, UserIdMixin, DiscussIdMixin {
        SubType sub_type = SubType::DEFAULT; // 默认事件子类型

        DiscussMessageEvent(int64_t message_id, std::string &&message, int32_t font, int64_t user_id,
                            int64_t discuss_id)
            : MessageEvent(Target(user_id, discuss_id, Target::DISCUSS), DetailType::DISCUSS, message_id,
                           std::move(message), font),
              UserIdMixin{user_id},
              DiscussIdMixin{discuss_id} {
        }
    };

    // 群文件上传事件
    struct GroupUploadEvent final : NoticeEvent, UserIdMixin, GroupIdMixin {
        SubType sub_type = SubType::DEFAULT; // 默认事件子类型
        File file; // 文件信息

        GroupUploadEvent(int64_t user_id, int64_t group_id, File &&file)
            : NoticeEvent(Target(user_id, group_id, Target::GROUP), DetailType::GROUP_UPLOAD),
              UserIdMixin{user_id},
              GroupIdMixin{group_id},
              file(std::move(file)) {
        }
    };

    // 群管理员变动事件
    struct GroupAdminEvent final : NoticeEvent, UserIdMixin, GroupIdMixin {
        enum class SubType {
            UNSET = 1, // 取消管理员
            SET = 2, // 设置管理员
        };

        SubType sub_type; // 事件子类型

        GroupAdminEvent(int64_t user_id, int64_t group_id, SubType sub_type)
            : NoticeEvent(Target(user_id, group_id, Target::GROUP), DetailType::GROUP_ADMIN),
              UserIdMixin{user_id},
              GroupIdMixin{group_id},
              sub_type(sub_type) {
        }
    };

    // 群成员减少事件
    struct GroupMemberDecreaseEvent final : NoticeEvent, UserIdMixin, GroupIdMixin, OperatorIdMixin {
        enum class SubType {
            LEAVE = 1, // 群成员退群
            KICK = 2, // 群成员被踢
            KICK_ME = 3, // 自己(登录号)被踢
        };

        SubType sub_type; // 事件子类型

        GroupMemberDecreaseEvent(int64_t user_id, int64_t group_id, int64_t operator_id, SubType sub_type)
            : NoticeEvent(Target(user_id, group_id, Target::GROUP), DetailType::GROUP_MEMBER_DECREASE),
              UserIdMixin{user_id},
              GroupIdMixin{group_id},
              OperatorIdMixin{operator_id},
              sub_type(sub_type) {
        }
    };

    // 群成员增加事件
    struct GroupMemberIncreaseEvent final : NoticeEvent, UserIdMixin, GroupIdMixin, OperatorIdMixin {
        enum class SubType {
            APPROVE = 1, // 管理员同意
            INVITE = 2, // 管理员邀请
        };

        SubType sub_type; // 事件子类型

        GroupMemberIncreaseEvent(int64_t user_id, int64_t group_id, int64_t operator_id, SubType sub_type)
            : NoticeEvent(Target(user_id, group_id, Target::GROUP), DetailType::GROUP_MEMBER_INCREASE),
              UserIdMixin{user_id},
              GroupIdMixin{group_id},
              OperatorIdMixin{operator_id},
              sub_type(sub_type) {
        }
    };

    // 群禁言事件
    struct GroupBanEvent final : NoticeEvent, UserIdMixin, GroupIdMixin, OperatorIdMixin {
        enum class SubType {
            LIFT_BAN = 1, // 解除禁言
            BAN = 2, // 禁言
        };

        SubType sub_type; // 事件子类型
        int64_t duration; // 禁言时长(秒)

        GroupBanEvent(int64_t user_id, int64_t group_id, int64_t operator_id, SubType sub_type, int64_t duration)
            : NoticeEvent(Target(user_id, group_id, Target::GROUP), DetailType::GROUP_BAN),
              UserIdMixin{user_id},
              GroupIdMixin{group_id},
              OperatorIdMixin{operator_id},
              sub_type(sub_type),
              duration(duration) {
        }
    };

    // 好友添加事件
    struct FriendAddEvent final : NoticeEvent, UserIdMixin {
        SubType sub_type = SubType::DEFAULT; // 默认事件子类型

        FriendAddEvent(int64_t user_id) : NoticeEvent(Target(user_id), DetailType::FRIEND_ADD), UserIdMixin{user_id} {
        }
    };

    // 好友请求事件
    struct FriendRequestEvent final : RequestEvent, UserIdMixin {
        SubType sub_type = SubType::DEFAULT; // 默认事件子类型

        FriendRequestEvent(std::string &&comment, Flag &&flag, int64_t user_id)
            : RequestEvent(Target(user_id), DetailType::FRIEND, std::move(comment), std::move(flag)),
              UserIdMixin{user_id} {
        }
    };

    // 群请求事件
    struct GroupRequestEvent final : RequestEvent, UserIdMixin, GroupIdMixin {
        enum class SubType {
            ADD = 1, // 他人申请入群
            INVITE = 2, // 自己(登录号)受邀入群
        };

        SubType sub_type; // 事件子类型

        GroupRequestEvent(std::string &&comment, Flag &&flag, int64_t user_id, int64_t group_id, SubType sub_type)
            : RequestEvent(Target(user_id, group_id, Target::GROUP), DetailType::GROUP, std::move(comment),
                           std::move(flag)),
              UserIdMixin{user_id},
              GroupIdMixin{group_id},
              sub_type(sub_type) {
        }
    };
} // namespace cq

namespace std {
    inline string to_string(cq::UserEvent::Type type) {
        using Type = cq::UserEvent::Type;
        switch (type) {
        case Type::MESSAGE:
            return "message";
        case Type::NOTICE:
            return "notice";
        case Type::REQUEST:
            return "request";
        default:
            return "unknown";
        }
    }

    inline string to_string(cq::MessageEvent::DetailType detail_type) {
        using DetailType = cq::MessageEvent::DetailType;
        switch (detail_type) {
        case DetailType::PRIVATE:
            return "private";
        case DetailType::GROUP:
            return "group";
        case DetailType::DISCUSS:
            return "discuss";
        default:
            return "unknown";
        }
    }

    inline string to_string(cq::NoticeEvent::DetailType detail_type) {
        using DetailType = cq::NoticeEvent::DetailType;
        switch (detail_type) {
        case DetailType::GROUP_UPLOAD:
            return "group_upload";
        case DetailType::GROUP_ADMIN:
            return "group_admin";
        case DetailType::GROUP_MEMBER_DECREASE:
            return "group_member_decrease";
        case DetailType::GROUP_MEMBER_INCREASE:
            return "group_member_increase";
        case DetailType::GROUP_BAN:
            return "group_ban";
        case DetailType::FRIEND_ADD:
            return "friend_add";
        default:
            return "unknown";
        }
    }

    inline string to_string(cq::RequestEvent::Operation operation) {
        using Operation = cq::RequestEvent::Operation;
        switch (operation) {
        case Operation::APPROVE:
            return "approve";
        case Operation::REJECT:
            return "reject";
        default:
            return "unknown";
        }
    }

    inline string to_string(cq::RequestEvent::DetailType detail_type) {
        using DetailType = cq::RequestEvent::DetailType;
        switch (detail_type) {
        case DetailType::FRIEND:
            return "friend";
        case DetailType::GROUP:
            return "group";
        default:
            return "unknown";
        }
    }

    inline string to_string(const cq::RequestEvent::Flag &flag) {
        return string(flag);
    }

    inline string to_string(cq::UserEvent::SubType sub_type) {
        return "default";
    }

    inline string to_string(cq::PrivateMessageEvent::SubType sub_type) {
        using SubType = cq::PrivateMessageEvent::SubType;
        switch (sub_type) {
        case SubType::FRIEND:
            return "friend";
        case SubType::GROUP:
            return "group";
        case SubType::DISCUSS:
            return "discuss";
        case SubType::OTHER:
            return "other";
        default:
            return "unknown";
        }
    }

    inline string to_string(cq::GroupAdminEvent::SubType sub_type) {
        using SubType = cq::GroupAdminEvent::SubType;
        switch (sub_type) {
        case SubType::UNSET:
            return "unset";
        case SubType::SET:
            return "set";
        default:
            return "unknown";
        }
    }

    inline string to_string(cq::GroupMemberDecreaseEvent::SubType sub_type) {
        using SubType = cq::GroupMemberDecreaseEvent::SubType;
        switch (sub_type) {
        case SubType::LEAVE:
            return "leave";
        case SubType::KICK:
            return "kick";
        case SubType::KICK_ME:
            return "kick_me";
        default:
            return "unknown";
        }
    }

    inline string to_string(cq::GroupMemberIncreaseEvent::SubType sub_type) {
        using SubType = cq::GroupMemberIncreaseEvent::SubType;
        switch (sub_type) {
        case SubType::APPROVE:
            return "approve";
        case SubType::INVITE:
            return "invite";
        default:
            return "unknown";
        }
    }

    inline string to_string(cq::GroupBanEvent::SubType sub_type) {
        using SubType = cq::GroupBanEvent::SubType;
        switch (sub_type) {
        case SubType::LIFT_BAN:
            return "lift_ban";
        case SubType::BAN:
            return "ban";
        default:
            return "unknown";
        }
    }

    inline string to_string(cq::GroupRequestEvent::SubType sub_type) {
        using SubType = cq::GroupRequestEvent::SubType;
        switch (sub_type) {
        case SubType::ADD:
            return "add";
        case SubType::INVITE:
            return "invite";
        default:
            return "unknown";
        }
    }
} // namespace std
