#include "./dir.h"

#include <filesystem>

#include "../utils/string.h"
#include "./api.h"

using namespace std;
namespace fs = std::filesystem;

namespace cq::dir {
    bool create_dir_if_not_exists(const string &dir) {
        const auto w_dir = utils::s2ws(dir);
        if (!fs::exists(w_dir)) {
            return fs::create_directories(w_dir);
        }
        return true;
    }

    string root() {
        return get_coolq_root_directory();
    }

    string app(const std::string &sub_dir_name) {
        if (sub_dir_name.empty()) {
            return get_app_directory();
        }
        const auto dir = fs::path(get_app_directory()) / (sub_dir_name.empty() ? "" : sub_dir_name) / "";
        create_dir_if_not_exists(dir.string());
        return dir.string();
    }

    std::string app_per_account(const std::string &sub_dir_name) {
        const auto dir = fs::path(get_app_directory()) / to_string(get_login_user_id())
                         / (sub_dir_name.empty() ? "" : sub_dir_name) / "";
        create_dir_if_not_exists(dir.string());
        return dir.string();
    }
} // namespace cq::dir
