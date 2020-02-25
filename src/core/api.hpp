#pragma once

#include "common.hpp"

#include "event.hpp"
#include "exception.hpp"
#include "type.hpp"

namespace cq {
    // API 调用失败
    struct ApiError : RuntimeError {
        ApiError(const int code)
            : RuntimeError("failed to call coolq api, error code: " + to_string(code)), code(code) {
        }

        const int code; // 错误码

        static const auto INVALID_DATA = 100; // 酷Q返回的数据无效
        static const auto INVALID_TARGET = 101; // 发送目标无效
        static const auto INVALID_ARGS = 102; // 参数无效
    };

    void _init_api();

    // 发送私聊消息
    int64_t send_private_message(const int64_t user_id, const std::string &message) noexcept(false);
    // 发送群消息
    int64_t send_group_message(const int64_t group_id, const std::string &message) noexcept(false);
    // 发送讨论组消息
    int64_t send_discuss_message(const int64_t discuss_id, const std::string &message) noexcept(false);

    // 向 target 指定的目标发送消息
    inline int64_t send_message(const Target &target, const std::string &message,
                                const bool at_user = false) noexcept(false) {
        if (target.group_id.has_value()) {
            if (at_user && target.user_id.has_value()) {
                return send_group_message(target.group_id.value(),
                                          "[CQ:at,qq=" + to_string(target.user_id.value()) + "] " + message);
            }
            return send_group_message(target.group_id.value(), message);
        }
        if (target.discuss_id.has_value()) {
            if (at_user && target.user_id.has_value()) {
                return send_discuss_message(target.discuss_id.value(),
                                            "[CQ:at,qq=" + to_string(target.user_id.value()) + "] " + message);
            }
            return send_discuss_message(target.discuss_id.value(), message);
        }
        if (target.user_id.has_value()) {
            return send_private_message(target.user_id.value(), message);
        }
        throw ApiError(ApiError::INVALID_TARGET);
    }

    // 撤回消息(可撤回自己 2 分钟内发的消息和比自己更低权限的群成员发的消息)
    void delete_message(const int64_t message_id) noexcept(false);

    // 发送好友赞
    void send_like(const int64_t user_id, const int32_t times = 1) noexcept(false);

    // 群踢人
    void set_group_kick(const int64_t group_id, const int64_t user_id,
                        const bool reject_future_request) noexcept(false);
    // 群禁言成员
    void set_group_ban(const int64_t group_id, const int64_t user_id, const int64_t duration) noexcept(false);
    // 群禁言匿名用户
    void set_group_anonymous_ban(const int64_t group_id, const Anonymous &anonymous,
                                 const int64_t duration) noexcept(false);
    // 群全体禁言
    void set_group_whole_ban(const int64_t group_id, const bool enable) noexcept(false);
    // 群设置管理员
    void set_group_admin(const int64_t group_id, const int64_t user_id, const bool enable) noexcept(false);
    // 群开关匿名
    void set_group_anonymous(const int64_t group_id, const bool enable) noexcept(false);
    // 群设置成员名片(群昵称)
    void set_group_card(const int64_t group_id, const int64_t user_id, const std::string &card) noexcept(false);
    // 退出群(若自己是群主且 dismiss 是 true 则解散群)
    void set_group_leave(const int64_t group_id, const bool dismiss) noexcept(false);
    // 群设置成员专属头衔
    void set_group_special_title(const int64_t group_id, const int64_t user_id, const std::string &special_title,
                                 const int64_t duration) noexcept(false);
    // 退出讨论组
    void set_discuss_leave(const int64_t discuss_id) noexcept(false);

    // 处理好友请求
    void set_friend_request(const RequestEvent::Flag &flag, const RequestEvent::Operation operation,
                            const std::string &remark = "") noexcept(false);
    // 处理群请求
    void set_group_request(const RequestEvent::Flag &flag, const GroupRequestEvent::SubType &sub_type,
                           const RequestEvent::Operation operation, const std::string &reason = "") noexcept(false);

    // 获取登录号 ID (QQ 号)
    int64_t get_login_user_id() noexcept(false);
    // 获取登录号昵称
    std::string get_login_nickname() noexcept(false);
    // 获取陌生人信息
    User get_stranger_info(const int64_t user_id, const bool no_cache = false) noexcept(false);
    // 获取好友列表
    std::vector<Friend> get_friend_list() noexcept(false);
    // 获取群列表
    std::vector<Group> get_group_list() noexcept(false);
    // 获取群信息
    Group get_group_info(const int64_t group_id, const bool no_cache = false) noexcept(false);
    // 获取群成员列表
    std::vector<GroupMember> get_group_member_list(const int64_t group_id) noexcept(false);
    // 获取群成员信息
    GroupMember get_group_member_info(const int64_t group_id, const int64_t user_id,
                                      const bool no_cache = false) noexcept(false);

    // 获取登录号信息
    inline User get_login_info() noexcept(false) {
        return get_stranger_info(get_login_user_id());
    }

    // 获取 cookies
    std::string get_cookies(const std::string &domain = "") noexcept(false);
    // 获取 CSRF Token (即 bkn)
    int32_t get_csrf_token() noexcept(false);
    // 获取应用数据目录(结尾包含"\")
    std::string get_app_directory() noexcept(false);
    // 获取酷Q根目录(结尾包含"\")
    std::string get_coolq_root_directory() noexcept(false);

    // 获取图片文件
    std::string get_image(const std::string &file) noexcept(false);
    // 获取语音文件
    std::string get_record(const std::string &file, const std::string &out_format,
                           const bool full_path = true) noexcept(false);
    // 检查是否可以发送图片
    bool can_send_image() noexcept(false);
    // 检查是否可以发送语音
    bool can_send_record() noexcept(false);

    // 添加日志(建议使用 cq::logging 模块, 而不是直接使用此函数)
    void add_log(const int32_t level, const std::string &tag, const std::string &message) noexcept(false);
} // namespace cq
