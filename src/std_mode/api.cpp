/**
 * 本文件需要从 CQP.dll 加载函数地址, 依赖 Windows.h, 必须在 Windows 环境下使用 MSVC 编译.
 */

#include "../core/api.h"

#include <Windows.h>

#include "../core/init.h"
#include "../utils/string.h"

using namespace std;

namespace cq {
    static vector<function<void(HMODULE)>> api_func_initializers;

    static bool add_func_initializer(const function<void(HMODULE)> &initializer) {
        api_func_initializers.push_back(initializer);
        return true;
    }

    void __init_api() {
        const auto dll = GetModuleHandleW(L"CQP.dll");
        for (const auto &initializer : api_func_initializers) {
            initializer(dll);
        }
    }

    namespace raw {
#define FUNC(ReturnType, FuncName, ...)                                                              \
    typedef __declspec(dllimport) ReturnType(__stdcall *__CQ_##FuncName##_T)(__VA_ARGS__);           \
    __CQ_##FuncName##_T CQ_##FuncName;                                                               \
    static bool __dummy_CQ_##FuncName = add_func_initializer([](auto dll) {                          \
        CQ_##FuncName = reinterpret_cast<__CQ_##FuncName##_T>(GetProcAddress(dll, "CQ_" #FuncName)); \
    });
#include "./api_funcs.inc"
#undef FUNC
    } // namespace raw

    template <typename T, typename enable_if<is_integral<T>::value>::type * = 0>
    inline T chk(const T ret) {
        if (ret < 0) {
            throw ApiError(static_cast<int>(ret));
        }
        return ret;
    }

    template <typename T, typename enable_if<is_pointer<T>::value>::type * = 0>
    inline T chk(const T ret_ptr) {
        if (!ret_ptr) {
            throw ApiError(ApiError::INVALID_DATA);
        }
        return ret;
    }

    int64_t send_private_message(const int64_t user_id, const std::string &message) {
        const auto ret = raw::CQ_sendPrivateMsg(__ac, user_id, utils::string_to_coolq(message).c_str());
        return static_cast<int64_t>(chk(ret));
    }

    int64_t send_group_message(const int64_t group_id, const std::string &message) {
        const auto ret = raw::CQ_sendGroupMsg(__ac, group_id, utils::string_to_coolq(message).c_str());
        return static_cast<int64_t>(chk(ret));
    }

    int64_t send_discuss_message(const int64_t discuss_id, const std::string &message) {
        const auto ret = raw::CQ_sendDiscussMsg(__ac, discuss_id, utils::string_to_coolq(message).c_str());
        return static_cast<int64_t>(chk(ret));
    }
} // namespace cq
