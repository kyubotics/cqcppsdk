#pragma once

#include <cqcppsdk/cqcppsdk.h>

#include "anymap.hpp"
#include "current.hpp"
#include "handler.hpp"
#include "lifecycle.hpp"
#include "matcher.hpp"
#include "string.hpp"
#include "traits.hpp"

namespace dolores {
    inline void init() {
        cq::on_coolq_start(dolores::_startup);
        cq::on_enable(dolores::_startup);
        cq::on_disable(dolores::_shutdown);
        cq::on_coolq_exit(dolores::_shutdown);

        cq::on_message(dolores::run_handlers<cq::MessageEvent>);
        cq::on_notice(dolores::run_handlers<cq::NoticeEvent>);
        cq::on_request(dolores::run_handlers<cq::RequestEvent>);
    }
} // namespace dolores
