#include "dolores/anymap.hpp"

#include "catch.hpp"

TEST_CASE("StrAnyMap", "[anymap]") {
    using namespace dolores;

    StrAnyMap m;

    m["foo"] = std::string("foo");
    REQUIRE(m.size() == 1);
    m["bar"] = 42;
    REQUIRE(m.size() == 2);

    auto foo = std::any_cast<std::string>(m.at("foo"));
    REQUIRE(foo == "foo");
    REQUIRE(m.get<std::string>("foo") == "foo");

    REQUIRE_THROWS_AS(m.get<int>("bad bar"), std::out_of_range);
    REQUIRE_THROWS_AS(m.get<float>("bar"), std::bad_any_cast);

    REQUIRE_NOTHROW(m.get<int>("bad bar", 0));
    REQUIRE_NOTHROW(m.get<float>("bar", 0.0f));
    REQUIRE(m.get<int>("bad bar", 0) == 0);
}
