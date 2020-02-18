#pragma once

#include <cqcppsdk/cqcppsdk.h>

#include <type_traits>

namespace dolores {
    template <typename E>
    static constexpr auto is_derived_from_user_event_v =
        std::is_base_of_v<cq::UserEvent, std::decay_t<E>> && !std::is_same_v<cq::UserEvent, std::decay_t<E>>;

    namespace matchers {
#if __cpp_concepts
        template <class T>
        concept MatcherBase = requires(T matcher, const cq::MessageEvent &event, Session &session) {
            { matcher(event, session) }
            ->std::boolean;
        };

        template <class T>
        concept MessageMatcher = MatcherBase<T> &&requires(T matcher, const cq::Target &target,
                                                           std::string_view message, Session &session) {
            { matcher(target, message, session) }
            ->std::boolean;
        };
#endif
        inline namespace {
            template <class T>
            auto is_in_matchers_namespace(T &&)
#if __cpp_concepts
                requires MatcherBase<T>
#else
                -> std::enable_if_t<std::is_invocable_r_v<bool, T, const cq::MessageEvent &, Session &>, std::true_type>
#endif
            {
                return std::true_type();
            }
        } // namespace
    }

    std::false_type is_in_matchers_namespace(...);

    template <class Fn>
    static constexpr auto is_matcher_v = decltype(is_in_matchers_namespace(std::declval<Fn>()))::value;
} // namespace dolores
