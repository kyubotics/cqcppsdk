#pragma once

#include "./common.h"

#include "../utils/string.h"
#include "./api.h"

namespace cq::dir {
    namespace stdfs = std::filesystem;

    inline bool create_dir_if_not_exists(const std::string &dir) {
        const auto w_dir = cq::utils::s2ws(dir);
        if (!stdfs::exists(w_dir)) {
            return stdfs::create_directories(w_dir);
        }
        return true;
    }

    inline std::string root() {
        return cq::get_coolq_root_directory();
    }

    inline std::string app(const std::string &sub_dir_name = "") {
        if (sub_dir_name.empty()) {
            return cq::get_app_directory();
        }
        const auto dir = stdfs::path(cq::get_app_directory()) / (sub_dir_name.empty() ? "" : sub_dir_name) / "";
        create_dir_if_not_exists(dir.string());
        return dir.string();
    }

    inline std::string app_per_account(const std::string &sub_dir_name) {
        const auto dir = stdfs::path(cq::get_app_directory()) / std::to_string(cq::get_login_user_id())
                         / (sub_dir_name.empty() ? "" : sub_dir_name) / "";
        create_dir_if_not_exists(dir.string());
        return dir.string();
    }
} // namespace cq::dir
