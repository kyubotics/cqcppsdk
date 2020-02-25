#pragma once

#include <cqcppsdk/cqcppsdk.h>

namespace dolores::watashi {
    inline auto &_user_id() {
        static int64_t val = 0;
        if (val == 0) {
            val = cq::get_login_user_id();
        }
        return val;
    }

    inline int64_t user_id() {
        return _user_id();
    }

    inline auto &_nickname() {
        static std::string val;
        if (val.empty()) {
            val = cq::get_login_nickname();
        }
        return val;
    }

    inline std::string nickname() {
        return _nickname();
    }
} // namespace dolores::watashi
