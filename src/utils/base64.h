#pragma once

#include <string>

namespace cq::utils {
    std::string base64_encode(const unsigned char *bytes, unsigned int len);
    std::string base64_decode(const std::string &str);
} // namespace cq::utils
