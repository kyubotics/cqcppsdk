#pragma once

#include <functional>
#include <map>
#include <memory>
#include <string>
#include <type_traits>
#include <vector>

#include "current.hpp"
#include "matcher.hpp"
#include "string.hpp"
#include "traits.hpp"

namespace dolores {
    template <typename E, typename = std::enable_if_t<is_derived_from_user_event_v<E>>>
    class Handler {
    public:
        explicit Handler(std::function<void(Current<E> &current)> impl, std::shared_ptr<MatcherBase> matcher = nullptr)
            : _impl(std::move(impl)), _matcher(std::move(matcher)) {
        }

        bool match_condition(const E &event, Session &session) const {
            if (!_matcher) return true;
            return _matcher->match(event, session);
        }

        void run(Current<E> &current) const {
            if (!_impl) return;
            _impl(current);
        }

    private:
        std::shared_ptr<MatcherBase> _matcher;
        std::function<void(Current<E> &current)> _impl;
    };

    struct _HandlerMapWrapper {
        template <typename E, typename = std::enable_if_t<is_derived_from_user_event_v<E>>>
        using HandlerMap = std::map<std::string, std::shared_ptr<Handler<E>>>;

        template <typename E, typename = std::enable_if_t<is_derived_from_user_event_v<E>>>
        static auto &handlers() {
            if constexpr (std::is_base_of_v<cq::MessageEvent, E>) {
                static HandlerMap<cq::MessageEvent> message_handlers;
                return message_handlers;
            } else if constexpr (std::is_base_of_v<cq::NoticeEvent, E>) {
                static HandlerMap<cq::NoticeEvent> notice_handlers;
                return notice_handlers;
            } else { // std::is_base_of_v<cq::RequestEvent, E>
                static HandlerMap<cq::RequestEvent> request_handlers;
                return request_handlers;
            }
        }
    };

    template <typename E, typename = std::enable_if_t<is_derived_from_user_event_v<E>>>
    inline bool add_handler(const std::string &name, const std::shared_ptr<Handler<E>> &handler) {
        auto &handlers = _HandlerMapWrapper::handlers<E>();
        handlers[name] = handler;
        return true;
    }

    template <typename E, typename = std::enable_if_t<is_derived_from_user_event_v<E>>>
    inline void run_handlers(const E &event) {
        const auto &handlers = _HandlerMapWrapper::handlers<E>();
        for (const auto &[name, handler] : handlers) {
            if (string::startswith(name, "_")) continue;

            Session session;
            if (handler->match_condition(event, session)) {
                if constexpr (std::is_base_of_v<cq::MessageEvent, E>) {
                    Current<cq::MessageEvent> current(event, session);
                    handler->run(current);
                } else if constexpr (std::is_base_of_v<cq::NoticeEvent, E>) {
                    Current<cq::NoticeEvent> current(event, session);
                    handler->run(current);
                } else { // std::is_base_of_v<cq::RequestEvent, E>
                    Current<cq::RequestEvent> current(event, session);
                    handler->run(current);
                }
            }
        }
    }
} // namespace dolores

#define dolores_on_message(Name, ...)                                                                                 \
    static void __dummy_message_handler_##Name(dolores::Current<cq::MessageEvent> &);                                 \
    static const auto __dummy_message_handler_##Name##_res = dolores::add_handler(                                    \
        #Name,                                                                                                        \
        std::make_shared<dolores::Handler<cq::MessageEvent>>(__dummy_message_handler_##Name,                          \
                                                             std::make_shared<dolores::matchers::all>(__VA_ARGS__))); \
    static void __dummy_message_handler_##Name(dolores::Current<cq::MessageEvent> &current)

#define dolores_on_notice(Name, ...)                                                                                 \
    static void __dummy_notice_handler_##Name(dolores::Current<cq::NoticeEvent> &);                                  \
    static const auto __dummy_notice_handler_##Name##_res = dolores::add_handler(                                    \
        #Name,                                                                                                       \
        std::make_shared<dolores::Handler<cq::NoticeEvent>>(__dummy_notice_handler_##Name,                           \
                                                            std::make_shared<dolores::matchers::all>(__VA_ARGS__))); \
    static void __dummy_notice_handler_##Name(dolores::Current<cq::NoticeEvent> &current)

#define dolores_on_request(Name, ...)                                                                                 \
    static void __dummy_request_handler_##Name(dolores::Current<cq::RequestEvent> &);                                 \
    static const auto __dummy_request_handler_##Name##_res = dolores::add_handler(                                    \
        #Name,                                                                                                        \
        std::make_shared<dolores::Handler<cq::RequestEvent>>(__dummy_request_handler_##Name,                          \
                                                             std::make_shared<dolores::matchers::all>(__VA_ARGS__))); \
    static void __dummy_request_handler_##Name(dolores::Current<cq::RequestEvent> &current)
