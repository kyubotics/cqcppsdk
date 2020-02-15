#include "dolores/session.hpp"

#include "catch.hpp"

TEST_CASE("Session", "[session]") {
    using namespace dolores;

    Session m;
    m["foo"] = std::string("foo");
    REQUIRE(m.size() == 1);
    m["bar"] = 42;
    REQUIRE(m.size() == 2);
    auto foo = std::any_cast<std::string>(m.at("foo"));
    REQUIRE(foo == "foo");
}
