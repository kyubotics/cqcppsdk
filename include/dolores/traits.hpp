#pragma once

#include <cqcppsdk/cqcppsdk.h>

#include <type_traits>

namespace dolores {
    template <typename E>
    using enable_if_derived_from_user_event_t =
        std::enable_if_t<std::is_base_of_v<cq::UserEvent, E> && !std::is_same_v<cq::UserEvent, E>>;

    struct Condition;

    template <typename T1, typename T2 = Condition>
    using enable_if_is_condition_t =
        std::enable_if_t<std::is_base_of_v<Condition, T1> && std::is_base_of_v<Condition, T2>>;
} // namespace dolores
