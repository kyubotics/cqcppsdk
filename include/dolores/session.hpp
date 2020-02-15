#pragma once

#include <any>
#include <map>
#include <string>

namespace dolores {
    class Session : public std::map<std::string, std::any> {
    public:
    };
} // namespace dolores
