#pragma once

#include <string>

namespace cq::utils {
    std::string string_to_coolq(const std::string &str);
    std::string string_from_coolq(const std::string &str);

    std::string ws2s(const std::wstring &ws);
    std::wstring s2ws(const std::string &s);
} // namespace cq::utils

namespace std {
    inline string to_string(const string &val) { return val; }
    inline string to_string(const bool val) { return val ? "true" : "false"; }
} // namespace std
