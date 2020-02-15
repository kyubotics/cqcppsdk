#pragma once

#include <string_view>

namespace dolores::string {
    inline bool startswith(const std::string_view &sv, const std::string_view &prefix) {
        return sv.substr(0, prefix.length()) == prefix;
    }

    inline bool endswith(const std::string_view &sv, const std::string_view &suffix) {
        if (sv.length() < suffix.length()) return false;
        return sv.substr(sv.length() - suffix.length()) == suffix;
    }

    inline bool contains(const std::string_view &sv, const std::string_view &sub) {
        return sv.find(sub) != std::string_view ::npos;
    }

    template <class It, class End>
    inline std::string_view string_view_from(It first, End last) {
        const auto size = last - first;
        if (size <= 0) return std::string_view("");
        return std::string_view(&*first, size);
    }
} // namespace dolores::string
