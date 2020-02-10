#pragma once

#include <functional>

namespace cq::utils {
    template <typename ReturnType, typename... Params, typename... Args>
    inline ReturnType call_if_valid(const std::function<ReturnType(Params...)> &func, Args &&... args) {
        if (func) {
            return func(std::forward<Args>(args)...);
        }
        return {};
    }

    template <typename... Params, typename... Args>
    inline void call_if_valid(const std::function<void(Params...)> &func, Args &&... args) {
        if (func) {
            func(std::forward<Args>(args)...);
        }
    }

    template <typename Cont, typename... Args>
    inline void call_all(const Cont &funcs, Args &&... args) {
        for (const auto &f : funcs) {
            call_if_valid(f, std::forward<Args>(args)...);
        }
    }
} // namespace cq::utils
