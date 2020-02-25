#include "dolores/watashi.hpp"
#include "test_dolores.hpp"

#include "catch.hpp"

TEST_CASE("watashi::user_id", "[watashi]") {
    REQUIRE(dolores::watashi::user_id() == LOGIN_ID);
}

TEST_CASE("watashi::nickname", "[watashi]") {
    REQUIRE(!dolores::watashi::nickname().empty());
}
