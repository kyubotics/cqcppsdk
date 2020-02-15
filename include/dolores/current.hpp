#pragma once

#include <cqcppsdk/cqcppsdk.h>

#include <memory>
#include <string>
#include <type_traits>

#include "matcher.hpp"
#include "session.hpp"
#include "traits.hpp"

namespace dolores {
    template <typename E, typename = enable_if_derived_from_user_event_t<E>>
    struct CurrentBase {
        const E &event;
        Session &session;

        template <typename T, typename = std::enable_if_t<std::is_base_of_v<E, T>>>
        explicit CurrentBase(const T &event) : event(event) {
        }

        template <typename T, typename = std::enable_if_t<std::is_base_of_v<E, T>>>
        CurrentBase(const T &event, Session &session) : event(event), session(session) {
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

    template <typename E, typename = enable_if_derived_from_user_event_t<E>>
    struct Current : CurrentBase<E> {
        using CurrentBase<E>::CurrentBase;
    };

    template <>
    struct Current<cq::MessageEvent> : CurrentBase<cq::MessageEvent> {
        using CurrentBase<cq::MessageEvent>::CurrentBase;

        std::string command_name() const {
            if (session.count(matchers::command::ARGUMENT) == 0) {
                return "";
            }
            return std::any_cast<std::string>(session.at(matchers::command::NAME));
        }

        std::string command_argument() const {
            if (session.count(matchers::command::ARGUMENT) == 0) {
                return "";
            }
            const auto sv = std::any_cast<std::string_view>(session.at(matchers::command::ARGUMENT));
            return std::string(sv.cbegin(), sv.cend());
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
