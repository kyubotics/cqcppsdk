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
    template <typename E>
    class Handler {
        static_assert(is_derived_from_user_event_v<E>);
    public:
        virtual ~Handler() = 0;
        virtual bool match(const E &event, Session &session) const = 0;
        virtual void run(Current<E> &current)  = 0;
    };
    inline Handler::~Handler() = default;

    struct _HandlerVecWrapper {
        template <typename E, typename = std::enable_if_t<is_derived_from_user_event_v<E>>>
        using HandlerVec = std::vector<std::shared_ptr<Handler<E>>>;

        template <typename E, typename = std::enable_if_t<is_derived_from_user_event_v<E>>>
        static auto &handlers() {
            if constexpr (std::is_base_of_v<cq::MessageEvent, E>) {
                static HandlerVec<cq::MessageEvent> message_handlers;
                return message_handlers;
            } else if constexpr (std::is_base_of_v<cq::NoticeEvent, E>) {
                static HandlerVec<cq::NoticeEvent> notice_handlers;
                return notice_handlers;
            } else { // std::is_base_of_v<cq::RequestEvent, E>
                static HandlerVec<cq::RequestEvent> request_handlers;
                return request_handlers;
            }
        }
    };

    template <typename E, typename = std::enable_if_t<is_derived_from_user_event_v<E>>>
    inline bool add_handler(const std::shared_ptr<Handler<E>> &handler) {
        auto &handlers = _HandlerVecWrapper::handlers<E>();
        handlers.push_back(handler);
        return true;
    }

    template <typename E, typename = std::enable_if_t<is_derived_from_user_event_v<E>>>
    inline void run_handlers(const E &event) {
        const auto &handlers = _HandlerVecWrapper::handlers<E>();
        for (const auto &handler : handlers) {
            Session session;
            if (handler->match(event, session)) {
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

    template <typename E, typename RunFunc, typename AllMatcher>
    class HandlerImplementation : public Handler<E> {
        static_assert(is_derived_from_user_event_v<E>);
    public:
        explicit HandlerImplementation(Func func, AllMatcher matcher)
                : _func(std::move(func)), _matcher(std::move(matcher)) {
        }

        bool match(const E &event, Session &session) const override {
            return _matcher(event, session);
        }

        void run(Current<E> &current) const override {
            _func(current);
        }

    private:
        RunFunc _func;
        AllMatcher _matcher;
    };
} // namespace dolores

#define _DOLORES_UNIQUE_NAME_2(Name, Number) Name##Number
#define _DOLORES_UNIQUE_NAME(Name, Number) _DOLORES_UNIQUE_NAME_2(Name, Number)
#define _DOLORES_HANDLER_UNIQUE_NAME() _DOLORES_UNIQUE_NAME(__H_A_N_D_L_E_R__, __COUNTER__)

#define _DOLORES_MAKE_HANDLER_2(EventType, FuncName, ...)                                                  \
    static void FuncName(dolores::Current<EventType> &);                                                   \
    static const auto FuncName##_res = dolores::add_handler(std::make_shared<dolores::HandlerImplementation<EventType>>( \
        FuncName, dolores::matchers::all(__VA_ARGS__)));                                 \
    static void FuncName(dolores::Current<EventType> &current)
#define _DOLORES_MAKE_HANDLER(EventType, FuncName, ...) _DOLORES_MAKE_HANDLER_2(EventType, FuncName, __VA_ARGS__)

#define dolores_on_message(...) _DOLORES_MAKE_HANDLER(cq::MessageEvent, _DOLORES_HANDLER_UNIQUE_NAME(), __VA_ARGS__)
#define dolores_on_notice(...) _DOLORES_MAKE_HANDLER(cq::NoticeEvent, _DOLORES_HANDLER_UNIQUE_NAME(), __VA_ARGS__)
#define dolores_on_request(...) _DOLORES_MAKE_HANDLER(cq::RequestEvent, _DOLORES_HANDLER_UNIQUE_NAME(), __VA_ARGS__)
