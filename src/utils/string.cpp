#include "./string.h"

#include <codecvt>

using namespace std;

namespace cq::utils {
    // TODO
    std::string string_to_coolq(const std::string &str) { return str; }
    std::string string_from_coolq(const std::string &str) { return str; }

    string ws2s(const wstring &ws) { return wstring_convert<codecvt_utf8<wchar_t>, wchar_t>().to_bytes(ws); }
    wstring s2ws(const string &s) { return wstring_convert<codecvt_utf8<wchar_t>, wchar_t>().from_bytes(s); }
} // namespace cq::utils
