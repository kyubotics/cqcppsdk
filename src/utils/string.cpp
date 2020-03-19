#include "string.hpp"

#include "memory.hpp"

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

namespace cq::utils {
    std::string string_convert_encoding(const std::string &text, const std::string &from_enc, const std::string &to_enc,
                                        float capability_factor) {
#ifdef _CQ_STD_MODE
        // 正在使用 std 模式, 经过酷Q的字符串可使用 libiconv 转码
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

        std::string result;

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
        return text; // dev 模式下原样返回
#endif
    }

#ifdef WIN32
    // static std::shared_ptr<wchar_t> win32_mb2wchar(const unsigned code_page, const char *mb_str) {
    //     const auto len = MultiByteToWideChar(code_page, 0, mb_str, -1, nullptr, 0);
    //     auto c_wstr_p = make_shared_array<wchar_t>(len + 1);
    //     MultiByteToWideChar(code_page, 0, mb_str, -1, c_wstr_p.get(), len);
    //     return c_wstr_p;
    // }

    static std::shared_ptr<char> win32_wchar2mb(const unsigned code_page, const wchar_t *wchar_str) {
        const auto len = WideCharToMultiByte(code_page, 0, wchar_str, -1, nullptr, 0, nullptr, nullptr);
        auto c_str_p = make_shared_array<char>(len + 1);
        WideCharToMultiByte(code_page, 0, wchar_str, -1, c_str_p.get(), len, nullptr, nullptr);
        return c_str_p;
    }

    enum class Win32Encoding : unsigned {
        // https://msdn.microsoft.com/en-us/library/windows/desktop/dd317756.aspx

        ANSI = 0,
        UTF8 = 65001,
        GB2312 = 936,
        GB18030 = 54936,
    };

    static std::string win32_string_encode(const std::string &s, const Win32Encoding encoding) {
        return win32_wchar2mb(static_cast<unsigned>(encoding), s2ws(s).c_str()).get();
    }

    // static std::string win32_string_decode(const std::string &b, const Win32Encoding encoding) {
    //     return ws2s(std::wstring(win32_mb2wchar(static_cast<unsigned>(encoding), b.c_str()).get()));
    // }
#endif

    std::string ansi(const std::string &s) {
#ifdef WIN32
        return win32_string_encode(s, Win32Encoding::ANSI);
#else
        return s; // 如果不是 Windows, 则原样返回
#endif
    }
} // namespace cq::utils
