#pragma once

#include <cqcppsdk/cqcppsdk.h>

#include <type_traits>

namespace dolores {
    template <typename E>
    static constexpr auto is_derived_from_user_event_v =
        std::is_base_of_v<cq::UserEvent, std::decay_t<E>> && !std::is_same_v<cq::UserEvent, std::decay_t<E>>;

    std::false_type is_in_matchers_namespace(...);
    // another overload is in matcher.hpp, for ADL

    template <class Fn>
    static constexpr auto is_matcher_v = decltype(is_in_matchers_namespace(std::declval<Fn>()))::value;
} // namespace dolores
