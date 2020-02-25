#pragma once

#include <any>
#include <map>
#include <string>
#include <type_traits>

namespace dolores {
    class StrAnyMap : public std::map<std::string, std::any> {
    public:
        template <typename T>
        T get(const key_type &key) {
            return std::any_cast<T>(this->at(key));
        }

        template <typename T, typename DefVal, typename = std::enable_if_t<std::is_convertible_v<DefVal, T>>>
        T get(const key_type &key, DefVal &&default_val) {
            try {
                return get<T>(key);
            } catch (std::out_of_range &) {
            } catch (std::bad_any_cast &) {
            }
            return std::forward<DefVal>(default_val);
        }
    };
} // namespace dolores
