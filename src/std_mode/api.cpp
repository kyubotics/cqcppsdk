/**
 * 本文件需要从 CQP.dll 加载函数地址, 依赖 Windows.h, 必须在 Windows 环境下使用 MSVC 编译.
 */

#include "../core/api.h"

#include <Windows.h>

#include "../core/init.h"
#include "../utils/string.h"

using namespace std;
using cq::utils::string_to_coolq;
using cq::utils::string_from_coolq;

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
    inline T chk(const T res) {
        if (res < 0) {
            throw ApiError(static_cast<int>(res));
        }
        return res;
    }

    template <typename T, typename enable_if<is_pointer<T>::value>::type * = 0>
    inline T chk(const T res_ptr) {
        if (!res_ptr) {
            throw ApiError(ApiError::INVALID_DATA);
        }
        return res_ptr;
    }

#pragma region Message

    int64_t send_private_message(const int64_t user_id, const std::string &message) {
        return static_cast<int64_t>(chk(raw::CQ_sendPrivateMsg(__ac, user_id, string_to_coolq(message).c_str())));
    }

    int64_t send_group_message(const int64_t group_id, const std::string &message) {
        return static_cast<int64_t>(chk(raw::CQ_sendGroupMsg(__ac, group_id, string_to_coolq(message).c_str())));
    }

    int64_t send_discuss_message(const int64_t discuss_id, const std::string &message) {
        return static_cast<int64_t>(chk(raw::CQ_sendDiscussMsg(__ac, discuss_id, string_to_coolq(message).c_str())));
    }

    void delete_message(const int64_t message_id) { chk(raw::CQ_deleteMsg(__ac, message_id)); }

#pragma endregion

#pragma region Friend Operation

    void send_like(const int64_t user_id, const int32_t times) { chk(raw::CQ_sendLikeV2(__ac, user_id, times)); }

#pragma endregion

#pragma region Group Operation

    void set_group_kick(const int64_t group_id, const int64_t user_id, const bool reject_future_request) {
        chk(raw::CQ_setGroupKick(__ac, group_id, user_id, reject_future_request));
    }

    void set_group_ban(const int64_t group_id, const int64_t user_id, const int64_t duration) {
        chk(raw::CQ_setGroupBan(__ac, group_id, user_id, duration));
    }

    void set_group_anonymous_ban(const int64_t group_id, const std::string &anonymous_flag, const int64_t duration) {
        chk(raw::CQ_setGroupAnonymousBan(__ac, group_id, string_to_coolq(anonymous_flag).c_str(), duration));
    }

    void set_group_whole_ban(const int64_t group_id, const bool enable) {
        chk(raw::CQ_setGroupWholeBan(__ac, group_id, enable));
    }

    void set_group_admin(const int64_t group_id, const int64_t user_id, const bool enable) {
        chk(raw::CQ_setGroupAdmin(__ac, group_id, user_id, enable));
    }

    void set_group_anonymous(const int64_t group_id, const bool enable) {
        chk(raw::CQ_setGroupAnonymous(__ac, group_id, enable));
    }

    void set_group_card(const int64_t group_id, const int64_t user_id, const std::string &card) {
        chk(raw::CQ_setGroupCard(__ac, group_id, user_id, string_to_coolq(card).c_str()));
    }

    void set_group_leave(const int64_t group_id, const bool dismiss) {
        chk(raw::CQ_setGroupLeave(__ac, group_id, dismiss));
    }

    void set_group_special_title(const int64_t group_id, const int64_t user_id, const std::string &special_title,
                                 const int64_t duration) {
        chk(raw::CQ_setGroupSpecialTitle(__ac, group_id, user_id, string_to_coolq(special_title).c_str(), duration));
    }

    void set_discuss_leave(const int64_t discuss_id) { chk(raw::CQ_setDiscussLeave(__ac, discuss_id)); }

#pragma endregion

#pragma region Request

    void set_friend_request(const std::string &flag, const RequestEvent::Operation operation,
                            const std::string &remark) {
        chk(raw::CQ_setFriendAddRequest(
            __ac, string_to_coolq(flag).c_str(), static_cast<int32_t>(operation), string_to_coolq(remark).c_str()));
    }

    void set_group_request(const std::string &flag, const std::string &sub_type,
                           const RequestEvent::Operation operation, const std::string &reason) {
        int32_t sub_type_i = 0;
        if (sub_type == GroupRequestEvent::SubType::ADD) {
            sub_type_i = 1;
        } else if (sub_type == GroupRequestEvent::SubType::INVITE) {
            sub_type_i = 2;
        } else {
            throw ApiError(ApiError::INVALID_ARGS);
        }
        chk(raw::CQ_setGroupAddRequestV2(__ac,
                                         string_to_coolq(flag).c_str(),
                                         sub_type_i,
                                         static_cast<int32_t>(operation),
                                         string_to_coolq(reason).c_str()));
    }

#pragma endregion

#pragma region QQ Information

    int64_t get_login_user_id() { return raw::CQ_getLoginQQ(__ac); }

    std::string get_login_nickname() { return string_from_coolq(chk(raw::CQ_getLoginNick(__ac))); }

    User get_stranger_info(const int64_t user_id, const bool no_cache) {
        const auto b64 = string_from_coolq(chk(raw::CQ_getStrangerInfo(__ac, user_id, no_cache)));
        try {
            return ObjectHelper::from_base64<User>(b64);
        } catch (ParseError &) {
            throw ApiError(ApiError::INVALID_DATA);
        }
    }

    std::vector<Friend> get_friend_list() {
        const auto b64 = string_from_coolq(chk(raw::CQ_getFriendList(__ac, false)));
        try {
            return ObjectHelper::multi_from_base64<std::vector<Friend>>(b64);
        } catch (ParseError &) {
            throw ApiError(ApiError::INVALID_DATA);
        }
    }

    std::vector<Group> get_group_list() {
        const auto b64 = string_from_coolq(chk(raw::CQ_getGroupList(__ac)));
        try {
            return ObjectHelper::multi_from_base64<std::vector<Group>>(b64);
        } catch (ParseError &) {
            throw ApiError(ApiError::INVALID_DATA);
        }
    }

    Group get_group_info(const int64_t group_id, const bool no_cache) {
        const auto b64 = string_from_coolq(chk(raw::CQ_getGroupInfo(__ac, group_id, no_cache)));
        try {
            return ObjectHelper::from_base64<Group>(b64);
        } catch (ParseError &) {
            throw ApiError(ApiError::INVALID_DATA);
        }
    }

    std::vector<GroupMember> get_group_member_list(const int64_t group_id) {
        const auto b64 = string_from_coolq(chk(raw::CQ_getGroupMemberList(__ac, group_id)));
        try {
            return ObjectHelper::multi_from_base64<std::vector<GroupMember>>(b64);
        } catch (ParseError &) {
            throw ApiError(ApiError::INVALID_DATA);
        }
    }

    GroupMember get_group_member_info(const int64_t group_id, const int64_t user_id, const bool no_cache) {
        const auto b64 = string_from_coolq(chk(raw::CQ_getGroupMemberInfoV2(__ac, group_id, user_id, no_cache)));
        try {
            return ObjectHelper::from_base64<GroupMember>(b64);
        } catch (ParseError &) {
            throw ApiError(ApiError::INVALID_DATA);
        }
    }

#pragma endregion

#pragma region CoolQ

    std::string get_cookies(const std::string &domain) {
        return string_from_coolq(chk(raw::CQ_getCookiesV2(__ac, string_to_coolq(domain).c_str())));
    }

    int32_t get_csrf_token() { return chk(raw::CQ_getCsrfToken(__ac)); }

    std::string get_app_directory() { return string_from_coolq(chk(raw::CQ_getAppDirectory(__ac))); }

    std::string get_coolq_root_directory() {
        constexpr size_t size = 1024;
        wchar_t w_exec_path[size]{};
        GetModuleFileNameW(nullptr, w_exec_path, size); // 此调用可获取到 "C:\\Path\\To\\CQ\\CQA.exe"
        auto exec_path = utils::ws2s(w_exec_path);
        return exec_path.substr(0, exec_path.rfind("\\")) + "\\";
    }

    std::string get_image(const std::string &file) {
        return string_from_coolq(chk(raw::CQ_getImage(__ac, string_to_coolq(file).c_str())));
    }

    std::string get_record(const std::string &file, const std::string &out_format, const bool full_path) {
        return string_from_coolq(chk(
            full_path ? raw::CQ_getRecordV2(__ac, string_to_coolq(file).c_str(), string_to_coolq(out_format).c_str())
                      : raw::CQ_getRecord(__ac, string_to_coolq(file).c_str(), string_to_coolq(out_format).c_str())));
    }

    bool can_send_image() { return raw::CQ_canSendImage(__ac); }

    bool can_send_record() { return raw::CQ_canSendRecord(__ac); }

    void add_log(int32_t level, const std::string &tag, const std::string &message) {
        chk(raw::CQ_addLog(__ac, level, string_to_coolq(tag).c_str(), string_to_coolq(message).c_str()));
    }

#pragma endregion
} // namespace cq
