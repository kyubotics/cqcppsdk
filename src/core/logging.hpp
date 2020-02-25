#include "common.hpp"

#include "api.hpp"

namespace cq::logging {
    enum Level {
        DEBUG = 0,
        INFO = 10,
        INFOSUCCESS = 11,
        INFORECV = 12,
        INFOSEND = 13,
        WARNING = 20,
        ERROR = 30,
        FATAL = 40,
    };

    inline void log(const Level level, const std::string &tag, const std::string &msg) noexcept {
        try {
            add_log(static_cast<int32_t>(level), tag, msg);
        } catch (cq::ApiError &) {
            // 忽略日志错误
        }
    }

    inline void debug(const std::string &tag, const std::string &msg) noexcept {
        log(DEBUG, tag, msg);
    }

    inline void info(const std::string &tag, const std::string &msg) noexcept {
        log(INFO, tag, msg);
    }

    inline void info_success(const std::string &tag, const std::string &msg) noexcept {
        log(INFOSUCCESS, tag, msg);
    }

    inline void info_recv(const std::string &tag, const std::string &msg) noexcept {
        log(INFORECV, tag, msg);
    }

    inline void info_send(const std::string &tag, const std::string &msg) noexcept {
        log(INFOSEND, tag, msg);
    }

    inline void warning(const std::string &tag, const std::string &msg) noexcept {
        log(WARNING, tag, msg);
    }

    inline void error(const std::string &tag, const std::string &msg) noexcept {
        log(ERROR, tag, msg);
    }

    inline void fatal(const std::string &tag, const std::string &msg) noexcept {
        log(FATAL, tag, msg);
    }
} // namespace cq::logging
