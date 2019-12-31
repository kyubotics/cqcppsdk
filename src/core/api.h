#pragma once

#include "./common.h"

#include "./exception.h"

namespace cq {
    struct ApiError : RuntimeError {
        int code;
        ApiError(const int code) : RuntimeError("failed to call coolq api") { this->code = code; }

        static const auto INVALID_DATA = 100;
        static const auto INVALID_TARGET = 101;
    };

    void __init_api();

    int64_t send_private_message(const int64_t user_id, const std::string &message);
    int64_t send_group_message(const int64_t group_id, const std::string &message);
    int64_t send_discuss_message(const int64_t discuss_id, const std::string &message);
} // namespace cq
