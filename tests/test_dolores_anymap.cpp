#include "catch.hpp"
#include "dolores/anymap.hpp"

TEST_CASE("StrAnyMap", "[anymap]") {
    using namespace dolores;

    StrAnyMap m;
    m["foo"] = std::string("foo");
    REQUIRE(m.size() == 1);
    m["bar"] = 42;
    REQUIRE(m.size() == 2);
    auto foo = std::any_cast<std::string>(m.at("foo"));
    REQUIRE(foo == "foo");
}
