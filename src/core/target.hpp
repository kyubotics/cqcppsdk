#pragma once

#include "common.hpp"

namespace cq {
    // 用于表示触发事件的主体和指定消息发送目标
    struct Target {
        std::optional<int64_t> user_id;
        std::optional<int64_t> group_id;
        std::optional<int64_t> discuss_id;

        enum Type { USER, GROUP, DISCUSS };

        Target() = default;

        explicit Target(const int64_t user_or_group_or_discuss_id, const Type type = USER) {
            switch (type) {
            case USER:
                user_id = user_or_group_or_discuss_id;
                break;
            case GROUP:
                group_id = user_or_group_or_discuss_id;
                break;
            case DISCUSS:
                discuss_id = user_or_group_or_discuss_id;
            }
        }

        explicit Target(const int64_t user_id, const int64_t group_or_discuss_id, const Type type)
            : Target(group_or_discuss_id, type) {
            this->user_id = user_id;
        }

        // 用户
        static Target user(const int64_t user_id) {
            return Target(user_id, USER);
        }

        // 群
        static Target group(const int64_t group_id) {
            return Target(group_id, GROUP);
        }

        // 讨论组
        static Target discuss(const int64_t discuss_id) {
            return Target(discuss_id, DISCUSS);
        }

        // 判断主体是否来自私聊消息、私人通知、私人请求等
        bool is_private() const {
            return user_id.has_value() && !group_id.has_value() && !discuss_id.has_value();
        }

        // 判断主体是否来自群消息、群通知、群请求等
        bool is_group() const {
            return group_id.has_value() && !discuss_id.has_value();
        }

        // 判断主体是否来自讨论组
        bool is_discuss() const {
            return discuss_id.has_value() && !group_id.has_value();
        }
    };
} // namespace cq
