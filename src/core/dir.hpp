#pragma once

#include "common.hpp"

#include "api.hpp"

namespace cq::dir {
    namespace fs = std::filesystem;

    inline bool create_dir_if_not_exists(const fs::path &dir) {
        if (!fs::exists(dir)) {
            return fs::create_directories(dir);
        }
        return true;
    }

    inline bool create_dir_if_not_exists(const std::string &dir) {
        return create_dir_if_not_exists(fs::u8path(dir));
    }

    /**
     * 获取酷Q主目录, 结尾保证是路径分隔符 ('/' 或 '\').
     * 若目录不存在, 不会自动创建.
     */
    template <typename... S>
    inline std::string root(const S &... sub_paths) {
        auto p = fs::u8path(cq::get_coolq_root_directory());
        (p.append(utils::s2ws(sub_paths)), ...);
        p /= ""; // ensure the trailing sep
        return p.u8string();
    }

    /**
     * 获取应用主目录, 结尾保证是路径分隔符 ('/' 或 '\').
     * 若目录不存在, 会自动创建.
     */
    template <typename... S>
    inline std::string app(const S &... sub_paths) {
        auto p = fs::u8path(cq::get_app_directory());
        (p.append(utils::s2ws(sub_paths)), ...);
        p /= ""; // ensure the trailing sep
        create_dir_if_not_exists(p);
        return p.u8string();
    }

    /**
     * 获取应用的账号独立目录, 结尾保证是路径分隔符 ('/' 或 '\').
     * 若目录不存在, 会自动创建.
     */
    template <typename... S>
    inline std::string app_per_account(const S &... sub_paths) {
        auto p = fs::u8path(app(to_string(cq::get_login_user_id())));
        (p.append(utils::s2ws(sub_paths)), ...);
        p /= ""; // ensure the trailing sep
        create_dir_if_not_exists(p);
        return p.u8string();
    }
} // namespace cq::dir
