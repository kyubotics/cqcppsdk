#pragma once

#include <cqcppsdk/cqcppsdk.h>

#include <type_traits>

namespace dolores {
    template <typename E>
    static constexpr auto is_derived_from_user_event_v =
        std::is_base_of_v<cq::UserEvent, std::decay_t<E>> && !std::is_same_v<cq::UserEvent, std::decay_t<E>>;

    struct MatcherBase;

    template <typename T1, typename T2 = MatcherBase>
    static constexpr auto is_matcher_v =
        std::is_base_of_v<MatcherBase, std::decay_t<T1>>&& std::is_base_of_v<MatcherBase, std::decay_t<T2>>;

    struct MessageMatcher;

    template <typename T1, typename T2 = MessageMatcher>
    static constexpr auto is_message_matcher_v =
        std::is_base_of_v<MessageMatcher, std::decay_t<T1>>&& std::is_base_of_v<MessageMatcher, std::decay_t<T2>>;
} // namespace dolores
