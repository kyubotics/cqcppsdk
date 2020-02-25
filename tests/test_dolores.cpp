#include "test_dolores.hpp"
#include "cqcppsdk/cqcppsdk.h"

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

namespace cq {
    int64_t get_login_user_id() {
        return LOGIN_ID;
    }

    std::string get_login_nickname() {
        return "fake nickname";
    }

    GroupMember get_group_member_info(const int64_t group_id, const int64_t user_id, const bool no_cache) {
        GroupMember mem;
        switch (user_id) {
        case MEMBER_USER_ID:
            mem.role = GroupRole::MEMBER;
            break;
        case ADMIN_USER_ID:
            mem.role = GroupRole::ADMIN;
            break;
        case OWNER_USER_ID:
            mem.role = GroupRole::OWNER;
            break;
        default:
            break;
        }
        return mem;
    }
} // namespace cq
