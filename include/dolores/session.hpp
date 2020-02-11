#pragma once

#include <cqcppsdk/cqcppsdk.h>

#include <memory>
#include <type_traits>

#include "traits.hpp"

namespace dolores {
    template <typename E, typename = enable_if_derived_from_user_event_t<E>>
    struct BaseSession {
        const E &event;

        template <typename T, typename = std::enable_if_t<std::is_base_of_v<E, T>>>
        explicit BaseSession(const T &event) : event(event) {
        }

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
    struct Session : BaseSession<E> {
        using BaseSession<E>::BaseSession;
    };

    template <>
    struct Session<cq::RequestEvent> : BaseSession<cq::RequestEvent> {
        using BaseSession<cq::RequestEvent>::BaseSession;

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

    using MessageSession = Session<cq::MessageEvent>;
    using NoticeSession = Session<cq::NoticeEvent>;
    using RequestSession = Session<cq::RequestEvent>;
} // namespace dolores
