#include "catch.hpp"
#include "dolores/string.hpp"

TEST_CASE("string::startswith", "[string]") {
    using namespace dolores;

    auto s = std::string("hello, world");
    REQUIRE(string::startswith(s, "hello"));
    REQUIRE(string::startswith(s, ""));
    REQUIRE(!string::startswith(s, std::string("world")));

    s = std::string();
    REQUIRE(!string::startswith(s, "hello"));
    REQUIRE(string::startswith(s, std::string()));
}

TEST_CASE("string::endswith", "[string]") {
    using namespace dolores;

    auto s = std::string("hello, world");
    REQUIRE(string::endswith(s, "world"));
    REQUIRE(string::endswith(s, ""));
    REQUIRE(!string::endswith(s, std::string("hello")));

    s = std::string();
    REQUIRE(!string::endswith(s, "world"));
    REQUIRE(string::endswith(s, std::string()));
}

TEST_CASE("string::contains", "[string]") {
    using namespace dolores;

    auto s = std::string("hello, world");
    REQUIRE(string::contains(s, "llo"));
    REQUIRE(string::contains(s, ""));
    REQUIRE(!string::contains(s, std::string("foo")));

    s = std::string();
    REQUIRE(!string::contains(s, "foo"));
    REQUIRE(string::contains(s, std::string()));
}
