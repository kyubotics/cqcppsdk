#pragma once

#include <atomic>
#include <functional>
#include <vector>

namespace dolores {
    struct _LifecycleVars {
        static auto &running() {
            static std::atomic<bool> running = false;
            return running;
        }

        static auto &startup_callbacks() {
            static std::vector<std::function<void()>> _startup_callbacks;
            return _startup_callbacks;
        }

        static auto &shutdown_callbacks() {
            static std::vector<std::function<void()>> _shutdown_callbacks;
            return _shutdown_callbacks;
        }
    };

    inline bool is_running() {
        return _LifecycleVars::running();
    }

    inline void on_startup(const std::function<void()> &cb) {
        _LifecycleVars::startup_callbacks().push_back(cb);
    }

    inline void on_shutdown(const std::function<void()> &cb) {
        _LifecycleVars::shutdown_callbacks().push_back(cb);
    }

    inline void _startup() {
        if (is_running()) return;
        for (const auto &cb : _LifecycleVars::startup_callbacks()) {
            cb();
        }
        _LifecycleVars::running() = true;
    }

    inline void _shutdown() {
        if (!is_running()) return;
        for (const auto &cb : _LifecycleVars::shutdown_callbacks()) {
            cb();
        }
        _LifecycleVars::running() = false;
    }
} // namespace dolores
