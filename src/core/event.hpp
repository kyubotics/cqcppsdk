#pragma once

#include "common.hpp"

#include "target.hpp"
#include "type.hpp"

namespace cq {
    struct Event {
        time_t time; // 酷Q触发事件的时间

        Event() : time(std::time(nullptr)) {
        }

        virtual ~Event() = default;
        Event(Event &&) = default;
        Event &operator=(Event &&) = default;
        Event(const Event &) = default;
        Event &operator=(const Event &) = default;
    };

    // 保留备用
    struct SysEvent : Event {
        using Event::Event;
    };

    // 用户事件
    struct UserEvent : Event {
        enum class Type {
            MESSAGE, // 消息事件
            NOTICE, // 通知事件
            REQUEST, // 请求事件
        };

        Type type; // 事件类型
        int64_t user_id; // 触发事件的用户 ID (QQ 号), 例如消息发送者, 新入群者, 申请加好友者等
        Target target; // 触发事件的主体, 与 user_id 的区别在于包含了群组或讨论组 ID (如果有的话)

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

        UserEvent(Type type, int64_t user_id, Target &&target)
            : type(type), user_id(user_id), target(std::move(target)) {
        }
    };

    // 消息事件
    struct MessageEvent : UserEvent {
        enum class DetailType {
            PRIVATE, // 私聊消息
            GROUP, // 群消息(包括新式讨论组)
            DISCUSS, // 讨论组消息(仅旧式讨论组)
        };

        DetailType detail_type; // 事件详细类型
        int64_t message_id; // 消息 ID
        std::string message; // 消息内容
        int32_t font; // 字体, 此属性已经没有实际意义

        MessageEvent(int64_t user_id, Target &&target, DetailType detail_type, int64_t message_id,
                     std::string &&message, int32_t font)
            : UserEvent(Type::MESSAGE, user_id, std::move(target)),
              detail_type(detail_type),
              message_id(message_id),
              message(std::move(message)),
              font(font) {
        }
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

        NoticeEvent(int64_t user_id, Target &&target, DetailType detail_type)
            : UserEvent(Type::NOTICE, user_id, std::move(target)), detail_type(detail_type) {
        }
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

        RequestEvent(int64_t user_id, Target &&target, DetailType detail_type, std::string &&comment, Flag &&flag)
            : UserEvent(Type::REQUEST, user_id, std::move(target)),
              detail_type(detail_type),
              comment(std::move(comment)),
              flag(std::move(flag)) {
        }
    };

    struct GroupIdMixin {
        int64_t group_id; // 群 ID (群号)
    };

    struct DiscussIdMixin {
        int64_t discuss_id; // 讨论组 ID
    };

    struct OperatorIdMixin {
        int64_t operator_id; // 操作者 ID (QQ 号)
    };

    // 私聊消息事件
    struct PrivateMessageEvent final : MessageEvent {
        enum class SubType {
            FRIEND = 11, // 好友消息
            GROUP = 2, // 群临时会话
            DISCUSS = 3, // 讨论组临时会话
            OTHER = 1, // 陌生人消息
        };

        SubType sub_type; // 事件子类型

        PrivateMessageEvent(int64_t user_id, int64_t message_id, std::string &&message, int32_t font, SubType sub_type)
            : MessageEvent(user_id, Target(user_id), DetailType::PRIVATE, message_id, std::move(message), font),
              sub_type(sub_type) {
        }
    };

    // 群消息事件
    struct GroupMessageEvent final : MessageEvent, GroupIdMixin {
        SubType sub_type = SubType::DEFAULT; // 默认事件子类型
        Anonymous anonymous; // 匿名信息

        // 是否匿名消息
        bool is_anonymous() const {
            return !anonymous.name.empty();
        }

        GroupMessageEvent(int64_t user_id, int64_t message_id, std::string &&message, int32_t font, int64_t group_id,
                          Anonymous &&anonymous)
            : MessageEvent(user_id, Target(user_id, group_id, Target::GROUP), DetailType::GROUP, message_id,
                           std::move(message), font),
              GroupIdMixin{group_id},
              anonymous(std::move(anonymous)) {
        }
    };

    // 讨论组消息事件
    struct DiscussMessageEvent final : MessageEvent, DiscussIdMixin {
        SubType sub_type = SubType::DEFAULT; // 默认事件子类型

        DiscussMessageEvent(int64_t user_id, int64_t message_id, std::string &&message, int32_t font,
                            int64_t discuss_id)
            : MessageEvent(user_id, Target(user_id, discuss_id, Target::DISCUSS), DetailType::DISCUSS, message_id,
                           std::move(message), font),
              DiscussIdMixin{discuss_id} {
        }
    };

    // 群文件上传事件
    struct GroupUploadEvent final : NoticeEvent, GroupIdMixin {
        SubType sub_type = SubType::DEFAULT; // 默认事件子类型
        File file; // 文件信息

        GroupUploadEvent(int64_t user_id, int64_t group_id, File &&file)
            : NoticeEvent(user_id, Target(user_id, group_id, Target::GROUP), DetailType::GROUP_UPLOAD),
              GroupIdMixin{group_id},
              file(std::move(file)) {
        }
    };

    // 群管理员变动事件
    struct GroupAdminEvent final : NoticeEvent, GroupIdMixin {
        enum class SubType {
            UNSET = 1, // 取消管理员
            SET = 2, // 设置管理员
        };

        SubType sub_type; // 事件子类型

        GroupAdminEvent(int64_t user_id, int64_t group_id, SubType sub_type)
            : NoticeEvent(user_id, Target(user_id, group_id, Target::GROUP), DetailType::GROUP_ADMIN),
              GroupIdMixin{group_id},
              sub_type(sub_type) {
        }
    };

    // 群成员减少事件
    struct GroupMemberDecreaseEvent final : NoticeEvent, GroupIdMixin, OperatorIdMixin {
        enum class SubType {
            LEAVE = 1, // 群成员退群
            KICK = 2, // 群成员被踢
            KICK_ME = 3, // 自己(登录号)被踢
        };

        SubType sub_type; // 事件子类型

        GroupMemberDecreaseEvent(int64_t user_id, int64_t group_id, int64_t operator_id, SubType sub_type)
            : NoticeEvent(user_id, Target(user_id, group_id, Target::GROUP), DetailType::GROUP_MEMBER_DECREASE),
              GroupIdMixin{group_id},
              OperatorIdMixin{operator_id},
              sub_type(sub_type) {
        }
    };

    // 群成员增加事件
    struct GroupMemberIncreaseEvent final : NoticeEvent, GroupIdMixin, OperatorIdMixin {
        enum class SubType {
            APPROVE = 1, // 管理员同意
            INVITE = 2, // 管理员邀请
        };

        SubType sub_type; // 事件子类型

        GroupMemberIncreaseEvent(int64_t user_id, int64_t group_id, int64_t operator_id, SubType sub_type)
            : NoticeEvent(user_id, Target(user_id, group_id, Target::GROUP), DetailType::GROUP_MEMBER_INCREASE),
              GroupIdMixin{group_id},
              OperatorIdMixin{operator_id},
              sub_type(sub_type) {
        }
    };

    // 群禁言事件
    struct GroupBanEvent final : NoticeEvent, GroupIdMixin, OperatorIdMixin {
        enum class SubType {
            LIFT_BAN = 1, // 解除禁言
            BAN = 2, // 禁言
        };

        SubType sub_type; // 事件子类型
        int64_t duration; // 禁言时长(秒)

        GroupBanEvent(int64_t user_id, int64_t group_id, int64_t operator_id, SubType sub_type, int64_t duration)
            : NoticeEvent(user_id, Target(user_id, group_id, Target::GROUP), DetailType::GROUP_BAN),
              GroupIdMixin{group_id},
              OperatorIdMixin{operator_id},
              sub_type(sub_type),
              duration(duration) {
        }
    };

    // 好友添加事件
    struct FriendAddEvent final : NoticeEvent {
        SubType sub_type = SubType::DEFAULT; // 默认事件子类型

        FriendAddEvent(int64_t user_id) : NoticeEvent(user_id, Target(user_id), DetailType::FRIEND_ADD) {
        }
    };

    // 好友请求事件
    struct FriendRequestEvent final : RequestEvent {
        SubType sub_type = SubType::DEFAULT; // 默认事件子类型

        FriendRequestEvent(int64_t user_id, std::string &&comment, Flag &&flag)
            : RequestEvent(user_id, Target(user_id), DetailType::FRIEND, std::move(comment), std::move(flag)) {
        }
    };

    // 群请求事件
    struct GroupRequestEvent final : RequestEvent, GroupIdMixin {
        enum class SubType {
            ADD = 1, // 他人申请入群
            INVITE = 2, // 自己(登录号)受邀入群
        };

        SubType sub_type; // 事件子类型

        GroupRequestEvent(int64_t user_id, std::string &&comment, Flag &&flag, int64_t group_id, SubType sub_type)
            : RequestEvent(user_id, Target(user_id, group_id, Target::GROUP), DetailType::GROUP, std::move(comment),
                           std::move(flag)),
              GroupIdMixin{group_id},
              sub_type(sub_type) {
        }
    };
} // namespace cq

namespace cq {
    inline std::string to_string(cq::UserEvent::Type type) {
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

    inline std::string to_string(cq::MessageEvent::DetailType detail_type) {
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

    inline std::string to_string(cq::NoticeEvent::DetailType detail_type) {
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

    inline std::string to_string(cq::RequestEvent::Operation operation) {
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

    inline std::string to_string(cq::RequestEvent::DetailType detail_type) {
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

    inline std::string to_string(cq::UserEvent::SubType sub_type) {
        return "default";
    }

    inline std::string to_string(cq::PrivateMessageEvent::SubType sub_type) {
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

    inline std::string to_string(cq::GroupAdminEvent::SubType sub_type) {
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

    inline std::string to_string(cq::GroupMemberDecreaseEvent::SubType sub_type) {
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

    inline std::string to_string(cq::GroupMemberIncreaseEvent::SubType sub_type) {
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

    inline std::string to_string(cq::GroupBanEvent::SubType sub_type) {
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

    inline std::string to_string(cq::GroupRequestEvent::SubType sub_type) {
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
} // namespace cq
