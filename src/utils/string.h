#pragma once

#include <string>

namespace cq::utils {
    std::string ws2s(const std::wstring &ws);
    std::wstring s2ws(const std::string &s);

    std::string string_convert_encoding(const std::string &text, const std::string &from_enc, const std::string &to_enc,
                                        float capability_factor);

    inline std::string string_encode(const std::string &s, const std::string &encoding,
                                     float capability_factor = 2.0f) {
        return string_convert_encoding(s, "utf-8", encoding, capability_factor);
    }

    inline std::string string_decode(const std::string &b, const std::string &encoding,
                                     float capability_factor = 2.0f) {
        return string_convert_encoding(b, encoding, "utf-8", capability_factor);
    }

    inline std::string string_to_coolq(const std::string &str) {
        return string_encode(str, "gb18030");
    }

    inline std::string string_from_coolq(const std::string &str) {
        return string_decode(str, "gb18030");
    }
} // namespace cq::utils

namespace std {
    inline string to_string(const string &val) {
        return val;
    }

    inline string to_string(const bool val) {
        return val ? "true" : "false";
    }
} // namespace std
