/**
 * 本文件提供 api.h 的 mock 实现, 仅输出日志并返回模拟数据, 可在任何平台编译.
 */

#include "../core/api.h"

#include <filesystem>
#include <iostream>

#include "../core/dir.h"
#include "./mock.h"

using namespace std;
namespace fs = filesystem;

namespace cq {
    void __init_api() {
    }

#pragma region Message

    static int64_t message_id = 0;

    int64_t send_private_message(const int64_t user_id, const std::string &message) {
        cout << "send_private_message" << endl;
        cout << "  user_id: " << user_id << endl;
        cout << "  message: " << message << endl;
        return ++message_id;
    }

    int64_t send_group_message(const int64_t group_id, const std::string &message) {
        cout << "send_group_message" << endl;
        cout << "  group_id: " << group_id << endl;
        cout << "  message: " << message << endl;
        return ++message_id;
    }

    int64_t send_discuss_message(const int64_t discuss_id, const std::string &message) {
        cout << "send_discuss_message" << endl;
        cout << "  discuss_id: " << discuss_id << endl;
        cout << "  message: " << message << endl;
        return ++message_id;
    }

    void delete_message(const int64_t message_id) {
        cout << "delete_message" << endl;
        cout << "  message_id: " << message_id << endl;
    }

#pragma endregion

#pragma region Friend Operation

    void send_like(const int64_t user_id, const int32_t times) {
        cout << "send_like" << endl;
        cout << "  user_id: " << user_id << endl;
        cout << "  times: " << times << endl;
    }

#pragma endregion

#pragma region Group Operation

    void set_group_kick(const int64_t group_id, const int64_t user_id, const bool reject_future_request) {
        cout << "set_group_kick" << endl;
        cout << "  group_id: " << group_id << endl;
        cout << "  user_id: " << user_id << endl;
        cout << "  reject_future_request: " << reject_future_request << endl;
    }

    void set_group_ban(const int64_t group_id, const int64_t user_id, const int64_t duration) {
        cout << "set_group_ban" << endl;
        cout << "  group_id: " << group_id << endl;
        cout << "  user_id: " << user_id << endl;
        cout << "  duration: " << duration << endl;
    }

    void set_group_anonymous_ban(const int64_t group_id, const std::string &anonymous_flag, const int64_t duration) {
        cout << "set_group_anonymous_ban" << endl;
        cout << "  group_id: " << group_id << endl;
        cout << "  anonymous_flag: " << anonymous_flag << endl;
        cout << "  duration: " << duration << endl;
    }

    void set_group_whole_ban(const int64_t group_id, const bool enable) {
        cout << "set_group_whole_ban" << endl;
        cout << "  group_id: " << group_id << endl;
        cout << "  enable: " << enable << endl;
    }

    void set_group_admin(const int64_t group_id, const int64_t user_id, const bool enable) {
        cout << "set_group_admin" << endl;
        cout << "  group_id: " << group_id << endl;
        cout << "  user_id: " << user_id << endl;
        cout << "  enable: " << enable << endl;
    }

    void set_group_anonymous(const int64_t group_id, const bool enable) {
        cout << "set_group_anonymous" << endl;
        cout << "  group_id: " << group_id << endl;
        cout << "  enable: " << enable << endl;
    }

    void set_group_card(const int64_t group_id, const int64_t user_id, const std::string &card) {
        cout << "set_group_card" << endl;
        cout << "  group_id: " << group_id << endl;
        cout << "  user_id: " << user_id << endl;
        cout << "  card: " << card << endl;
    }

    void set_group_leave(const int64_t group_id, const bool dismiss) {
        cout << "set_group_leave" << endl;
        cout << "  group_id: " << group_id << endl;
        cout << "  dismiss: " << dismiss << endl;
    }

    void set_group_special_title(const int64_t group_id, const int64_t user_id, const std::string &special_title,
                                 const int64_t duration) {
        cout << "set_group_special_title" << endl;
        cout << "  group_id: " << group_id << endl;
        cout << "  user_id: " << user_id << endl;
        cout << "  special_title: " << special_title << endl;
        cout << "  duration: " << duration << endl;
    }

    void set_discuss_leave(const int64_t discuss_id) {
        cout << "set_discuss_leave" << endl;
        cout << "  discuss_id: " << discuss_id << endl;
    }

#pragma endregion

#pragma region Request

    void set_friend_request(const std::string &flag, const RequestEvent::Operation operation,
                            const std::string &remark) {
        cout << "set_friend_request" << endl;
        cout << "  flag: " << flag << endl;
        cout << "  operation: " << operation << endl;
        cout << "  remark: " << remark << endl;
    }

    void set_group_request(const std::string &flag, const std::string &sub_type,
                           const RequestEvent::Operation operation, const std::string &reason) {
        cout << "set_group_request" << endl;
        cout << "  flag: " << flag << endl;
        cout << "  sub_type: " << sub_type << endl;
        cout << "  operation: " << operation << endl;
        cout << "  reason: " << reason << endl;
    }

#pragma endregion

#pragma region QQ Information

    int64_t get_login_user_id() {
        cout << "get_login_user_id" << endl;
        return FAKE_LOGIN_USER_ID;
    }

    std::string get_login_nickname() {
        cout << "get_login_nickname" << endl;
        return FAKE_LOGIN_NICKNAME;
    }

    User get_stranger_info(const int64_t user_id, const bool no_cache) {
        cout << "get_stranger_info" << endl;
        cout << "  user_id: " << user_id << endl;
        cout << "  no_cache: " << no_cache << endl;
        User u;
        u.user_id = user_id;
        u.nickname = FAKE_OTHER_NICKNAME;
        u.sex = Sex::MALE;
        u.age = 20;
        return u;
    }

    std::vector<Friend> get_friend_list() {
        cout << "get_friend_list" << endl;
        Friend f;
        f.user_id = FAKE_OTHER_USER_ID;
        f.nickname = FAKE_OTHER_NICKNAME;
        f.remark = FAKE_NAME;
        return {f};
    }

    std::vector<Group> get_group_list() {
        cout << "get_group_list" << endl;
        return {};
    }

    Group get_group_info(const int64_t group_id, const bool no_cache) {
        cout << "get_group_info" << endl;
        cout << "  no_cache: " << no_cache << endl;
        return Group();
    }

    std::vector<GroupMember> get_group_member_list(const int64_t group_id) {
        cout << "get_group_member_list" << endl;
        cout << "  group_id: " << group_id << endl;
        return {};
    }

    GroupMember get_group_member_info(const int64_t group_id, const int64_t user_id, const bool no_cache) {
        cout << "get_group_member_info" << endl;
        cout << "  group_id: " << group_id << endl;
        cout << "  user_id: " << user_id << endl;
        cout << "  no_cache: " << no_cache << endl;
        return GroupMember();
    }

#pragma endregion

#pragma region CoolQ

    std::string get_cookies(const std::string &domain) {
        cout << "get_cookies" << endl;
        return "uin=" + to_string(FAKE_LOGIN_USER_ID);
    }

    int32_t get_csrf_token() {
        cout << "get_csrf_token" << endl;
        return 123;
    }

    std::string get_app_directory() {
        cout << "get_app_directory" << endl;
        const auto app_dir = fs::path(get_coolq_root_directory()) / "data" / "app" / "";
        dir::create_dir_if_not_exists(app_dir.string());
        return app_dir.string();
    }

    std::string get_coolq_root_directory() {
        cout << "get_coolq_root_directory" << endl;
        string root_dir = FAKE_COOLQ_ROOT_DIR;
        dir::create_dir_if_not_exists(root_dir);
        return fs::absolute(root_dir).string();
    }

    std::string get_image(const std::string &file) {
        cout << "get_image" << endl;
        return "";
    }

    std::string get_record(const std::string &file, const std::string &out_format, const bool full_path) {
        cout << "get_record" << endl;
        return "";
    }

    bool can_send_image() {
        cout << "can_send_image" << endl;
        return false;
    }

    bool can_send_record() {
        cout << "can_send_record" << endl;
        return false;
    }

    void add_log(const int32_t level, const std::string &tag, const std::string &message) {
        cout << "add_log" << endl;
        cout << "  level: " << level << endl;
        cout << "  tag: " << tag << endl;
        cout << "  message: " << message << endl;
    }

#pragma endregion
} // namespace cq
