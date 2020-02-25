/**
 * 本文件调用了所有 api.h 中声明的 API, 以便在链接时发现 api.cpp 中遗漏或签名不一致的错误.
 * 本文件应该在 std 模式和 dev 模式加入链接; test 模式下测试编写者应该按需实现 API, 而不应链接此文件.
 */

#include "core/api.hpp"

using namespace cq;

namespace cq {
    void __api_test() {
        send_private_message(0, "");
        send_group_message(0, "");
        send_discuss_message(0, "");
        delete_message(0);

        send_like(0, 0);

        set_group_kick(0, 0, false);
        set_group_ban(0, 0, 0);
        set_group_anonymous_ban(0, Anonymous(), 0);
        set_group_whole_ban(0, false);
        set_group_admin(0, 0, false);
        set_group_anonymous(0, false);
        set_group_card(0, 0, "");
        set_group_leave(0, false);
        set_group_special_title(0, 0, "", 0);
        set_discuss_leave(0);

        set_friend_request(RequestEvent::Flag{""}, RequestEvent::Operation::APPROVE, "");
        set_group_request(
            RequestEvent::Flag{""}, GroupRequestEvent::SubType::ADD, RequestEvent::Operation::APPROVE, "");

        get_login_user_id();
        get_login_nickname();
        get_stranger_info(0, false);
        get_friend_list();
        get_group_list();
        get_group_info(0, false);
        get_group_member_list(0);
        get_group_member_info(0, 0, false);

        get_cookies("");
        get_csrf_token();
        get_app_directory();
        get_coolq_root_directory();

        get_image("");
        get_record("", "", false);
        can_send_image();
        can_send_record();
    }
} // namespace cq
