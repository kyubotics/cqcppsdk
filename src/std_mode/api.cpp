/**
 * 本文件需要从 CQP.dll 加载函数地址, 依赖 Windows.h, 必须在 Windows 环境下使用 MSVC 编译.
 */

#include "../core/api.hpp"
#include "../core/init.hpp"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

using std::enable_if_t;
using std::is_integral_v;
using std::is_pointer_v;
using cq::utils::string_to_coolq;
using cq::utils::string_from_coolq;

namespace cq {
    namespace raw {
        using cq_bool_t = int32_t;

#define FUNC(ReturnType, FuncName, ...) static ReturnType(__stdcall *CQ_##FuncName)(__VA_ARGS__) = nullptr;
#include "./api_funcs.inc"
#undef FUNC
    } // namespace raw

    void _init_api() {
        const auto dll = GetModuleHandleW(L"CQP.dll");

#define FUNC(ReturnType, FuncName, ...) \
    raw::CQ_##FuncName = reinterpret_cast<decltype(raw::CQ_##FuncName)>(GetProcAddress(dll, "CQ_" #FuncName));
#include "./api_funcs.inc"
#undef FUNC
    }

    template <typename T, enable_if_t<is_integral_v<T>> * = nullptr>
    inline decltype(auto) chk(T &&res) noexcept(false) {
        if (res < 0) {
            throw ApiError(static_cast<int>(res));
        }
        return std::forward<T>(res);
    }

    template <typename T, enable_if_t<is_pointer_v<T>> * = nullptr>
    inline decltype(auto) chk(T &&res_ptr) noexcept(false) {
        if (!res_ptr) {
            throw ApiError(ApiError::INVALID_DATA);
        }
        return std::forward<T>(res_ptr);
    }

#pragma region Message

    int64_t send_private_message(const int64_t user_id, const std::string &message) {
        return static_cast<int64_t>(chk(raw::CQ_sendPrivateMsg(_ac(), user_id, string_to_coolq(message).c_str())));
    }

    int64_t send_group_message(const int64_t group_id, const std::string &message) {
        return static_cast<int64_t>(chk(raw::CQ_sendGroupMsg(_ac(), group_id, string_to_coolq(message).c_str())));
    }

    int64_t send_discuss_message(const int64_t discuss_id, const std::string &message) {
        return static_cast<int64_t>(chk(raw::CQ_sendDiscussMsg(_ac(), discuss_id, string_to_coolq(message).c_str())));
    }

    void delete_message(const int64_t message_id) {
        chk(raw::CQ_deleteMsg(_ac(), message_id));
    }

#pragma endregion

#pragma region Friend Operation

    void send_like(const int64_t user_id, const int32_t times) {
        chk(raw::CQ_sendLikeV2(_ac(), user_id, times));
    }

#pragma endregion

#pragma region Group Operation

    void set_group_kick(const int64_t group_id, const int64_t user_id, const bool reject_future_request) {
        chk(raw::CQ_setGroupKick(_ac(), group_id, user_id, reject_future_request));
    }

    void set_group_ban(const int64_t group_id, const int64_t user_id, const int64_t duration) {
        chk(raw::CQ_setGroupBan(_ac(), group_id, user_id, duration));
    }

    void set_group_anonymous_ban(const int64_t group_id, const Anonymous &anonymous, const int64_t duration) {
        chk(raw::CQ_setGroupAnonymousBan(_ac(), group_id, anonymous.base64.c_str(), duration));
    }

    void set_group_whole_ban(const int64_t group_id, const bool enable) {
        chk(raw::CQ_setGroupWholeBan(_ac(), group_id, enable));
    }

    void set_group_admin(const int64_t group_id, const int64_t user_id, const bool enable) {
        chk(raw::CQ_setGroupAdmin(_ac(), group_id, user_id, enable));
    }

    void set_group_anonymous(const int64_t group_id, const bool enable) {
        chk(raw::CQ_setGroupAnonymous(_ac(), group_id, enable));
    }

    void set_group_card(const int64_t group_id, const int64_t user_id, const std::string &card) {
        chk(raw::CQ_setGroupCard(_ac(), group_id, user_id, string_to_coolq(card).c_str()));
    }

    void set_group_leave(const int64_t group_id, const bool dismiss) {
        chk(raw::CQ_setGroupLeave(_ac(), group_id, dismiss));
    }

    void set_group_special_title(const int64_t group_id, const int64_t user_id, const std::string &special_title,
                                 const int64_t duration) {
        chk(raw::CQ_setGroupSpecialTitle(_ac(), group_id, user_id, string_to_coolq(special_title).c_str(), duration));
    }

    void set_discuss_leave(const int64_t discuss_id) {
        chk(raw::CQ_setDiscussLeave(_ac(), discuss_id));
    }

#pragma endregion

#pragma region Request

    void set_friend_request(const RequestEvent::Flag &flag, const RequestEvent::Operation operation,
                            const std::string &remark) {
        chk(raw::CQ_setFriendAddRequest(
            _ac(), string_to_coolq(flag).c_str(), static_cast<int32_t>(operation), string_to_coolq(remark).c_str()));
    }

    void set_group_request(const RequestEvent::Flag &flag, const GroupRequestEvent::SubType &sub_type,
                           const RequestEvent::Operation operation, const std::string &reason) {
        chk(raw::CQ_setGroupAddRequestV2(_ac(),
                                         string_to_coolq(flag).c_str(),
                                         static_cast<int32_t>(sub_type),
                                         static_cast<int32_t>(operation),
                                         string_to_coolq(reason).c_str()));
    }

#pragma endregion

#pragma region QQ Information

    int64_t get_login_user_id() {
        return chk(raw::CQ_getLoginQQ(_ac()));
    }

    std::string get_login_nickname() {
        return string_from_coolq(chk(raw::CQ_getLoginNick(_ac())));
    }

    User get_stranger_info(const int64_t user_id, const bool no_cache) {
        try {
            return ObjectHelper::from_base64<User>(chk(raw::CQ_getStrangerInfo(_ac(), user_id, no_cache)));
        } catch (ParseError &) {
            throw ApiError(ApiError::INVALID_DATA);
        }
    }

    std::vector<Friend> get_friend_list() {
        try {
            return ObjectHelper::multi_from_base64<std::vector<Friend>>(chk(raw::CQ_getFriendList(_ac(), false)));
        } catch (ParseError &) {
            throw ApiError(ApiError::INVALID_DATA);
        }
    }

    std::vector<Group> get_group_list() {
        try {
            return ObjectHelper::multi_from_base64<std::vector<Group>>(chk(raw::CQ_getGroupList(_ac())));
        } catch (ParseError &) {
            throw ApiError(ApiError::INVALID_DATA);
        }
    }

    Group get_group_info(const int64_t group_id, const bool no_cache) {
        try {
            return ObjectHelper::from_base64<Group>(chk(raw::CQ_getGroupInfo(_ac(), group_id, no_cache)));
        } catch (ParseError &) {
            throw ApiError(ApiError::INVALID_DATA);
        }
    }

    std::vector<GroupMember> get_group_member_list(const int64_t group_id) {
        try {
            return ObjectHelper::multi_from_base64<std::vector<GroupMember>>(
                chk(raw::CQ_getGroupMemberList(_ac(), group_id)));
        } catch (ParseError &) {
            throw ApiError(ApiError::INVALID_DATA);
        }
    }

    GroupMember get_group_member_info(const int64_t group_id, const int64_t user_id, const bool no_cache) {
        try {
            return ObjectHelper::from_base64<GroupMember>(
                chk(raw::CQ_getGroupMemberInfoV2(_ac(), group_id, user_id, no_cache)));
        } catch (ParseError &) {
            throw ApiError(ApiError::INVALID_DATA);
        }
    }

#pragma endregion

#pragma region CoolQ

    std::string get_cookies(const std::string &domain) {
        return string_from_coolq(chk(raw::CQ_getCookiesV2(_ac(), string_to_coolq(domain).c_str())));
    }

    int32_t get_csrf_token() {
        return chk(raw::CQ_getCsrfToken(_ac()));
    }

    std::string get_app_directory() {
        return string_from_coolq(chk(raw::CQ_getAppDirectory(_ac())));
    }

    std::string get_coolq_root_directory() {
        constexpr size_t size = 1024;
        wchar_t w_exec_path[size]{};
        GetModuleFileNameW(nullptr, w_exec_path, size); // 此调用可获取到 "C:\\Path\\To\\CQ\\CQA.exe"
        auto exec_path = utils::ws2s(w_exec_path);
        return exec_path.substr(0, exec_path.rfind('\\')) + '\\';
    }

    std::string get_image(const std::string &file) {
        return string_from_coolq(chk(raw::CQ_getImage(_ac(), string_to_coolq(file).c_str())));
    }

    std::string get_record(const std::string &file, const std::string &out_format, const bool full_path) {
        return string_from_coolq(chk(
            full_path ? raw::CQ_getRecordV2(_ac(), string_to_coolq(file).c_str(), string_to_coolq(out_format).c_str())
                      : raw::CQ_getRecord(_ac(), string_to_coolq(file).c_str(), string_to_coolq(out_format).c_str())));
    }

    bool can_send_image() {
        return raw::CQ_canSendImage(_ac());
    }

    bool can_send_record() {
        return raw::CQ_canSendRecord(_ac());
    }

    void add_log(const int32_t level, const std::string &tag, const std::string &message) {
        chk(raw::CQ_addLog(_ac(), level, string_to_coolq(tag).c_str(), string_to_coolq(message).c_str()));
    }

#pragma endregion
} // namespace cq
