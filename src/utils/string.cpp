#include "./string.h"

#include <codecvt>
#include <locale>

#if defined(_CQ_STD_MODE)
#include <Windows.h>
#endif

using namespace std;

namespace cq::utils {
    string ws2s(const wstring &ws) {
        return wstring_convert<codecvt_utf8<wchar_t>, wchar_t>().to_bytes(ws);
    }

    wstring s2ws(const string &s) {
        return wstring_convert<codecvt_utf8<wchar_t>, wchar_t>().from_bytes(s);
    }

    std::string string_convert_encoding(const std::string &text, const std::string &from_enc, const std::string &to_enc,
                                        float capability_factor) {
#if defined(_CQ_STD_MODE)
        // 正在使用标准模式, 经过酷Q的字符串可使用 libiconv 转码
        using iconv_t = void *;
        static HMODULE iconv_dll = nullptr;
        static iconv_t (*iconv_open)(const char *, const char *) = nullptr;
        static size_t (*iconv)(iconv_t cd, char **, size_t *, char **, size_t *) = nullptr;
        static int (*iconv_close)(iconv_t) = nullptr;

        if (!iconv_dll) {
            iconv_dll = LoadLibraryW(L"libiconv.dll");
            iconv_open =
                reinterpret_cast<iconv_t (*)(const char *, const char *)>(GetProcAddress(iconv_dll, "libiconv_open"));
            iconv = reinterpret_cast<size_t (*)(iconv_t cd, char **, size_t *, char **, size_t *)>(
                GetProcAddress(iconv_dll, "libiconv"));
            iconv_close = reinterpret_cast<int (*)(iconv_t)>(GetProcAddress(iconv_dll, "libiconv_close"));
        }

        string result;

        const auto cd = iconv_open(to_enc.c_str(), from_enc.c_str());
        auto in = const_cast<char *>(text.data());
        auto in_bytes_left = text.size();

        if (in_bytes_left == 0) {
            return result;
        }

        auto out_bytes_left =
            static_cast<decltype(in_bytes_left)>(static_cast<double>(in_bytes_left) * capability_factor);
        auto out = new char[out_bytes_left]{0};
        const auto out_begin = out;

        try {
            if (static_cast<size_t>(-1) != iconv(cd, &in, &in_bytes_left, &out, &out_bytes_left)) {
                // successfully converted
                result = out_begin;
            }
        } catch (...) {
        }

        delete[] out_begin;
        iconv_close(cd);

        return result;
#else
        return text;
#endif
    }
} // namespace cq::utils
