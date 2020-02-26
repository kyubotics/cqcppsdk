#pragma once

#include <algorithm>
#include <cctype>
#include <codecvt>
#include <functional>
#include <limits>
#include <locale>
#include <string>

namespace cq::utils {
    inline std::string ws2s(const std::wstring &ws) {
        return std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t>().to_bytes(ws);
    }

    inline std::wstring s2ws(const std::string &s) {
        return std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t>().from_bytes(s);
    }

    std::string ansi(const std::string &s);

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

    inline void string_replace(std::string &str, const std::string &old_val, const std::string &new_val) {
        // see https://stackoverflow.com/a/29752943

        std::string res;
        res.reserve(str.length()); // avoids a few memory allocations

        std::string::size_type last_pos = 0;
        std::string::size_type pos;
        while (std::string::npos != (pos = str.find(old_val, last_pos))) {
            res.append(str, last_pos, pos - last_pos);
            res += new_val;
            last_pos = pos + old_val.length();
        }

        // care for the rest after last occurrence
        res += str.substr(last_pos);
        str.swap(res);
    }

    inline int isspace_s(int ch) {
        return ch >= 0 && ch <= std::numeric_limits<unsigned char>::max() ? std::isspace(ch) : 0;
    }

    inline void string_ltrim(std::string &s) {
        // see https://stackoverflow.com/a/217605
        s.erase(s.begin(), std::find_if(s.begin(), s.end(), std::not_fn(isspace_s)));
    }

    inline void string_rtrim(std::string &s) {
        s.erase(std::find_if(s.rbegin(), s.rend(), std::not_fn(isspace_s)).base(), s.end());
    }

    inline void string_trim(std::string &s) {
        string_ltrim(s);
        string_rtrim(s);
    }
} // namespace cq::utils

namespace cq {
    using std::to_string;

    template <class T, typename = std::enable_if_t<std::is_convertible_v<T &&, std::string>>>
    inline auto to_string(T &&val) {
        return static_cast<std::string>(std::forward<T>(val));
    }

    template <class T, typename = std::enable_if_t<std::is_same_v<typename std::decay<T>::type, bool>, std::string>>
    inline auto to_string(T val) {
        return val ? "true" : "false";
    }
} // namespace cq
