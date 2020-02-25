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
        explicit Handler(std::function<void(Current<E> &current)> func, std::string name,
                         std::shared_ptr<MatcherBase> matcher = nullptr)
            : _func(std::move(func)), _name(std::move(name)), _matcher(std::move(matcher)) {
        }

        std::string name() const {
            return _name;
        }

        bool match(const E &event, StrAnyMap &matcher_data) const {
            if (!_matcher) return true;
            return _matcher->match(event, matcher_data);
        }

        void run(Current<E> &current) const {
            if (!_func) return;
            _func(current);
        }

    private:
        std::function<void(Current<E> &current)> _func;
        std::string _name;
        std::shared_ptr<MatcherBase> _matcher;
    };

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
            StrAnyMap matcher_data;
            if (handler->match(event, matcher_data)) {
                try {
                    if constexpr (std::is_base_of_v<cq::MessageEvent, E>) {
                        Current<cq::MessageEvent> current(event, matcher_data);
                        handler->run(current);
                    } else if constexpr (std::is_base_of_v<cq::NoticeEvent, E>) {
                        Current<cq::NoticeEvent> current(event, matcher_data);
                        handler->run(current);
                    } else { // std::is_base_of_v<cq::RequestEvent, E>
                        Current<cq::RequestEvent> current(event, matcher_data);
                        handler->run(current);
                    }
                } catch (std::exception &err) {
                    cq::logging::error("Dolores",
                                       "事件处理程序 " + handler->name() + " 运行时抛出异常, "
                                           + "异常类型: " + typeid(err).name() + ", 异常信息: " + err.what());
                }
            }
        }
    }
} // namespace dolores

#define _DOLORES_UNIQUE_NAME_2(Name, Number) Name##Number
#define _DOLORES_UNIQUE_NAME(Name, Number) _DOLORES_UNIQUE_NAME_2(Name, Number)
#define _DOLORES_HANDLER_UNIQUE_NAME() _DOLORES_UNIQUE_NAME(__H_A_N_D_L_E_R__, __COUNTER__)

#define _DOLORES_MAKE_HANDLER_2(EventType, FuncName, Name, ...)                                            \
    static void FuncName(dolores::Current<EventType> &);                                                   \
    static const auto FuncName##_res = dolores::add_handler(std::make_shared<dolores::Handler<EventType>>( \
        FuncName, Name, std::make_shared<dolores::matchers::all>(__VA_ARGS__)));                           \
    static void FuncName(dolores::Current<EventType> &current)
#define _DOLORES_MAKE_HANDLER(EventType, FuncName, Name, ...) \
    _DOLORES_MAKE_HANDLER_2(EventType, FuncName, Name, __VA_ARGS__)

#define dolores_on_message(Name, ...) \
    _DOLORES_MAKE_HANDLER(cq::MessageEvent, _DOLORES_HANDLER_UNIQUE_NAME(), Name, __VA_ARGS__)
#define dolores_on_notice(Name, ...) \
    _DOLORES_MAKE_HANDLER(cq::NoticeEvent, _DOLORES_HANDLER_UNIQUE_NAME(), Name, __VA_ARGS__)
#define dolores_on_request(Name, ...) \
    _DOLORES_MAKE_HANDLER(cq::RequestEvent, _DOLORES_HANDLER_UNIQUE_NAME(), Name, __VA_ARGS__)
