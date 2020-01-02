#pragma once

#include "./common.h"

namespace cq::dir {
    bool create_dir_if_not_exists(const std::string &dir);

    std::string root();
    std::string app(const std::string &sub_dir_name = "");
    std::string app_per_account(const std::string &sub_dir_name);
} // namespace cq::dir
