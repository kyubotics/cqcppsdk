#pragma once

#include <cqcppsdk/cqcppsdk.h>

#include <memory>
#include <string>
#include <type_traits>

#include "anymap.hpp"
#include "matcher.hpp"
#include "traits.hpp"

namespace dolores {
    template <typename E>
    struct Current;

    template <typename E, typename = std::enable_if_t<is_derived_from_user_event_v<E>>>
    struct CurrentBase {
        const E &event;
        StrAnyMap &matcher_data;

        template <typename T, typename = std::enable_if_t<std::is_base_of_v<E, T>>>
        explicit CurrentBase(const T &event) : event(event) {
        }

        template <typename T, typename = std::enable_if_t<std::is_base_of_v<E, T>>>
        CurrentBase(const T &event, StrAnyMap &matcher_data) : event(event), matcher_data(matcher_data) {
        }

        virtual ~CurrentBase() = default;

        template <typename T, typename = std::enable_if_t<std::is_base_of_v<E, T>>>
        auto event_as() const {
            return dynamic_cast<const std::decay_t<T> &>(event);
        }

        int64_t send(const std::string &message, const bool at_user = false) const {
            return cq::send_message(event.target, message, at_user);
        }

        int64_t reply(const std::string &message) const {
            return send(message, true);
        }
    };

    template <typename E>
    struct Current : CurrentBase<E> {
        using CurrentBase<E>::CurrentBase;
    };

    template <>
    struct Current<cq::MessageEvent> : CurrentBase<cq::MessageEvent> {
        using CurrentBase<cq::MessageEvent>::CurrentBase;

        std::string command_starter() const {
            return std::string(matcher_data.get<std::string_view>(matchers::command::STARTER, ""));
        }

        std::string command_name() const {
            return std::string(matcher_data.get<std::string_view>(matchers::command::NAME, ""));
        }

        std::string command_argument() const {
            return std::string(matcher_data.get<std::string_view>(matchers::command::ARGUMENT, ""));
        }
    };

    template <>
    struct Current<cq::RequestEvent> : CurrentBase<cq::RequestEvent> {
        using CurrentBase<cq::RequestEvent>::CurrentBase;

        void approve() const {
            _set_request(true);
        }

        void reject() const {
            _set_request(false);
        }

    private:
        void _set_request(const bool approve = true) const {
            switch (event.detail_type) {
            case cq::RequestEvent::DetailType::FRIEND: {
                cq::set_friend_request(
                    event.flag, approve ? cq::RequestEvent::Operation::APPROVE : cq::RequestEvent::Operation::REJECT);
                break;
            }
            case cq::RequestEvent::DetailType::GROUP: {
                cq::set_group_request(
                    event.flag,
                    event_as<cq::GroupRequestEvent>().sub_type,
                    approve ? cq::RequestEvent::Operation::APPROVE : cq::RequestEvent::Operation::REJECT);
                break;
            }
            default:
                break;
            }
        }
    };
} // namespace dolores
