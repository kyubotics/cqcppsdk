#include "./common.h"

#include "../utils/string.h"
#include "./api.h"

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

    inline void log(const Level level, const std::string &tag, const std::string &msg) {
        add_log(static_cast<int32_t>(level), tag, msg);
    }

    inline void debug(const std::string &tag, const std::string &msg) {
        log(DEBUG, tag, msg);
    }

    inline void info(const std::string &tag, const std::string &msg) {
        log(INFO, tag, msg);
    }

    inline void info_success(const std::string &tag, const std::string &msg) {
        log(INFOSUCCESS, tag, msg);
    }

    inline void info_recv(const std::string &tag, const std::string &msg) {
        log(INFORECV, tag, msg);
    }

    inline void info_send(const std::string &tag, const std::string &msg) {
        log(INFOSEND, tag, msg);
    }

    inline void warning(const std::string &tag, const std::string &msg) {
        log(WARNING, tag, msg);
    }

    inline void error(const std::string &tag, const std::string &msg) {
        log(ERROR, tag, msg);
    }

    inline void fatal(const std::string &tag, const std::string &msg) {
        log(FATAL, tag, msg);
    }
} // namespace cq::logging
