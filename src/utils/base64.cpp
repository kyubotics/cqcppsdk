#include "base64.hpp"

#include "./vendor/cpp-base64/base64.h"

namespace cq::utils {
    std::string base64_encode(const unsigned char *bytes, const unsigned int len) {
        return ::base64_encode(bytes, len);
    }

    std::string base64_decode(const std::string &str) {
        return ::base64_decode(str);
    }
} // namespace cq::utils
