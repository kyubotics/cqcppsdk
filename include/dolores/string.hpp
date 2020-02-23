#pragma once

#include <string_view>
#include <string>

namespace dolores::string {
    constexpr bool startswith(std::string_view sv, std::string_view prefix) {
        return sv.substr(0, prefix.length()) == prefix;
    }

    constexpr bool endswith(std::string_view sv, std::string_view suffix) {
        if (sv.length() < suffix.length()) return false;
        return sv.substr(sv.length() - suffix.length()) == suffix;
    }

    constexpr bool contains(std::string_view sv, std::string_view sub) {
        return sv.find(sub) != std::string_view ::npos;
    }

    constexpr std::string_view string_view_from(std::string_view::const_iterator first, std::string_view::const_iterator last) {
        return std::string_view(&*first, std::distance(first, last));
    }

    inline std::string_view string_view_from(std::string::const_iterator first, std::string::const_iterator last) {
        return std::string_view(&*first, std::distance(first, last));
    }

} // namespace dolores::string
