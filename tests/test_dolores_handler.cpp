#include "cqcppsdk/cqcppsdk.h"
#include "dolores/handler.hpp"

#include "catch.hpp"

dolores_on_message("") {
}

dolores_on_message("") {
}

dolores_on_notice("") {
}

dolores_on_request("") {
}

TEST_CASE("on_* macros", "[handler]") {
    using namespace dolores;

    REQUIRE(_HandlerVecWrapper::handlers<cq::MessageEvent>().size() == 2);
    REQUIRE(_HandlerVecWrapper::handlers<cq::NoticeEvent>().size() == 1);
    REQUIRE(_HandlerVecWrapper::handlers<cq::RequestEvent>().size() == 1);
}
