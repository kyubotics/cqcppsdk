#pragma once

#include "common.hpp"

#include "api.hpp"

namespace cq::dir {
    namespace fs = std::filesystem;

    inline bool create_dir_if_not_exists(const std::string &dir) {
        const auto w_dir = cq::utils::s2ws(dir);
        if (!fs::exists(w_dir)) {
            return fs::create_directories(w_dir);
        }
        return true;
    }

    /**
     * 获取酷Q主目录, 结尾保证是路径分隔符 ('/' 或 '\').
     * 若目录不存在, 不会自动创建.
     */
    template <typename... S>
    inline std::string root(const S &... sub_paths) {
        auto p = fs::path(cq::get_coolq_root_directory());
        (p.append(sub_paths), ...);
        p /= ""; // ensure the trailing sep
        return p.string();
    }

    /**
     * 获取应用主目录, 结尾保证是路径分隔符 ('/' 或 '\').
     * 若目录不存在, 会自动创建.
     */
    template <typename... S>
    inline std::string app(const S &... sub_paths) {
        auto p = fs::path(cq::get_app_directory());
        (p.append(sub_paths), ...);
        p /= ""; // ensure the trailing sep
        create_dir_if_not_exists(p.string());
        return p.string();
    }

    /**
     * 获取应用的账号独立目录, 结尾保证是路径分隔符 ('/' 或 '\').
     * 若目录不存在, 会自动创建.
     */
    template <typename... S>
    inline std::string app_per_account(const S &... sub_paths) {
        auto p = fs::path(app(to_string(cq::get_login_user_id())));
        (p.append(sub_paths), ...);
        p /= ""; // ensure the trailing sep
        create_dir_if_not_exists(p.string());
        return p.string();
    }
} // namespace cq::dir
