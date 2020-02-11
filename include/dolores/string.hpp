#pragma once

#include <string>

namespace dolores::string {
    inline bool startswith(const std::string &s, const std::string &prefix) {
        return s.substr(0, prefix.length()) == prefix;
    }

    inline bool endswith(const std::string &s, const std::string &suffix) {
        return s.substr(s.length() - suffix.length()) == suffix;
    }

    inline bool contains(const std::string &s, const std::string &sub) {
        return s.find(sub) != std::string::npos;
    }
} // namespace dolores::string
