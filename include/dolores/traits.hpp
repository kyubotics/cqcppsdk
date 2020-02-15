#pragma once

#include <cqcppsdk/cqcppsdk.h>

#include <type_traits>

namespace dolores {
    template <typename E>
    using enable_if_derived_from_user_event_t = std::enable_if_t<
        std::is_base_of_v<cq::UserEvent, std::decay_t<E>> && !std::is_same_v<cq::UserEvent, std::decay_t<E>>>;

    struct MatcherBase;

    template <typename T1, typename T2 = MatcherBase>
    using enable_if_is_matcher_t = std::enable_if_t<
        std::is_base_of_v<MatcherBase, std::decay_t<T1>> && std::is_base_of_v<MatcherBase, std::decay_t<T2>>>;
} // namespace dolores
