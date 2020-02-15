#include "dolores/matcher.hpp"
#include "test_dolores.hpp"

#include "catch.hpp"

using namespace dolores;
using namespace dolores::matchers;

std::pair<cq::PrivateMessageEvent, Session> construct() {
    return {cq::PrivateMessageEvent(1, 1, "hello, world", 0, cq::PrivateMessageEvent::SubType::FRIEND), Session()};
}

template <typename Cond>
std::shared_ptr<MatcherBase> to_cond(Cond &&c) {
    return std::make_shared<std::decay_t<Cond>>(std::forward<Cond>(c));
}

TEST_CASE("matchers::Not", "[condition]") {
    auto [event, session] = construct();
    REQUIRE_FALSE(to_cond(Not(contains("ll")))->match(event, session));
    REQUIRE(to_cond(Not(contains("foo")))->match(event, session));
    REQUIRE(to_cond(!contains("foo"))->match(event, session));
}

TEST_CASE("matchers::And", "[condition]") {
    auto [event, session] = construct();
    REQUIRE(to_cond(And(contains("ll"), contains("wor")))->match(event, session));
    REQUIRE(to_cond((contains("ll") && contains("wor")))->match(event, session));
    REQUIRE_FALSE(to_cond((contains("foo") && contains("wor")))->match(event, session));
}

TEST_CASE("matchers::Or", "[condition]") {
    auto [event, session] = construct();
    REQUIRE(to_cond(Or(contains("ll"), contains("wor")))->match(event, session));
    REQUIRE(to_cond((contains("ll") || contains("foo")))->match(event, session));
    REQUIRE_FALSE(to_cond((contains("foo") || contains("bar")))->match(event, session));
}

TEST_CASE("matchers::All", "[condition]") {
    auto [event, session] = construct();
    REQUIRE(to_cond(All(contains("ll"), contains("wor"), contains(",")))->match(event, session));
    REQUIRE_FALSE(to_cond(All(contains("ll"), contains("wor"), contains("foo")))->match(event, session));
}

TEST_CASE("matchers::type", "[condition]") {
    auto [event, session] = construct();
    const auto &ev_ref = event;
    const auto &ev2_ref = static_cast<const cq::MessageEvent &>(ev_ref);
    REQUIRE(to_cond(type<cq::PrivateMessageEvent>)->match(ev_ref, session));
    REQUIRE(to_cond(type<cq::PrivateMessageEvent>)->match(ev2_ref, session));
    REQUIRE_FALSE(to_cond(type<cq::GroupMessageEvent>)->match(ev2_ref, session));
}

TEST_CASE("matchers::unblocked", "[condition]") {
    auto [event, session] = construct();
    REQUIRE(to_cond(unblocked())->match(event, session));
    event.block();
    REQUIRE_FALSE(to_cond(unblocked())->match(event, session));
}

TEST_CASE("matchers::startswith", "[condition]") {
    auto [event, session] = construct();
    REQUIRE(to_cond(startswith("hello"))->match(event, session));
    REQUIRE_FALSE(to_cond(startswith("foo"))->match(event, session));
}

TEST_CASE("matchers::endswith", "[condition]") {
    auto [event, session] = construct();
    REQUIRE(to_cond(endswith("world"))->match(event, session));
    REQUIRE_FALSE(to_cond(startswith("foo"))->match(event, session));
}

TEST_CASE("matchers::contains", "[condition]") {
    auto [event, session] = construct();
    REQUIRE(to_cond(contains("world"))->match(event, session));
    REQUIRE(to_cond(contains(","))->match(event, session));
    REQUIRE_FALSE(to_cond(contains("foo"))->match(event, session));
}

TEST_CASE("matchers::command", "[condition]") {
    auto [event, session] = construct();
    event.message = "/echo";
    REQUIRE(to_cond(command("echo"))->match(event, session));
    REQUIRE(session.count(command::STARTER) > 0);
    REQUIRE(session.count(command::NAME) > 0);
    REQUIRE(session.count(command::ARGUMENT) > 0);
    REQUIRE(session.get<std::string>(command::NAME) == "echo");

    event.message = "/echo   hello";
    REQUIRE(to_cond(command({"echo", "e"}))->match(event, session));
    REQUIRE(session.get<std::string_view>(command::ARGUMENT) == "  hello");

    event.message = "echo wow";
    REQUIRE(to_cond(command("echo", {"", "?"}))->match(event, session));

    event.message = "/e";
    REQUIRE_FALSE(to_cond(command("echo"))->match(event, session));
    event.message = "";
    REQUIRE_FALSE(to_cond(command("echo"))->match(event, session));
}

TEST_CASE("matchers::user", "[condition]") {
    auto [event, session] = construct();
    REQUIRE(to_cond(user())->match(event, session));
    REQUIRE(to_cond(user({event.user_id}))->match(event, session));
    REQUIRE_FALSE(to_cond(user({event.user_id + 1}))->match(event, session));
    REQUIRE(to_cond(user::exclude({event.user_id + 1}))->match(event, session));
    REQUIRE_FALSE(to_cond(user::exclude({event.user_id}))->match(event, session));

    auto ge = cq::GroupAdminEvent(1, 1, cq::GroupAdminEvent::SubType::SET);
    REQUIRE(to_cond(user())->match(ge, session));
}

TEST_CASE("matchers::direct", "[condition]") {
    auto [event, session] = construct();
    REQUIRE(to_cond(direct())->match(event, session));
    REQUIRE(to_cond(direct({event.user_id}))->match(event, session));
    REQUIRE_FALSE(to_cond(direct({event.user_id + 1}))->match(event, session));
    REQUIRE(to_cond(direct::exclude({event.user_id + 1}))->match(event, session));
    REQUIRE_FALSE(to_cond(direct::exclude({event.user_id}))->match(event, session));

    auto ge = cq::GroupAdminEvent(1, 1, cq::GroupAdminEvent::SubType::SET);
    REQUIRE_FALSE(to_cond(direct())->match(ge, session));
}

TEST_CASE("matchers::group", "[condition]") {
    auto [_, session] = construct();
    auto event = cq::GroupAdminEvent(1, 1, cq::GroupAdminEvent::SubType::SET);
    REQUIRE(to_cond(group())->match(event, session));
    REQUIRE(to_cond(group({event.group_id}))->match(event, session));
    REQUIRE_FALSE(to_cond(group({event.group_id + 1}))->match(event, session));
    REQUIRE(to_cond(group::exclude({event.group_id + 1}))->match(event, session));
    REQUIRE_FALSE(to_cond(group::exclude({event.group_id}))->match(event, session));

    REQUIRE_FALSE(to_cond(group())->match(construct().first, session));
}

TEST_CASE("matchers::discuss", "[condition]") {
    auto [_, session] = construct();
    auto event = cq::DiscussMessageEvent(1, 1, "hello", 0, 1);
    REQUIRE(to_cond(discuss())->match(event, session));

    REQUIRE_FALSE(to_cond(discuss())->match(construct().first, session));
}

TEST_CASE("matchers::group_roles", "[condition]") {
    auto [_, session] = construct();
    auto event = cq::GroupMessageEvent(MEMBER_USER_ID, 1, "hello", 0, 1, cq::Anonymous());
    REQUIRE(to_cond(group_roles({cq::GroupRole::MEMBER}))->match(event, session));
    REQUIRE_FALSE(to_cond(group_roles({cq::GroupRole::OWNER}))->match(event, session));

    REQUIRE(to_cond(group_roles({cq::GroupRole::OWNER}))->match(construct().first, session));
}

TEST_CASE("matchers::admin", "[condition]") {
    auto [_, session] = construct();
    auto event = cq::GroupMessageEvent(ADMIN_USER_ID, 1, "hello", 0, 1, cq::Anonymous());
    REQUIRE(to_cond(admin())->match(event, session));
    event.user_id = OWNER_USER_ID;
    REQUIRE(to_cond(admin())->match(event, session));
    event.user_id = MEMBER_USER_ID;
    REQUIRE_FALSE(to_cond(admin())->match(event, session));
}

TEST_CASE("matchers::owner", "[condition]") {
    auto [_, session] = construct();
    auto event = cq::GroupMessageEvent(OWNER_USER_ID, 1, "hello", 0, 1, cq::Anonymous());
    REQUIRE(to_cond(owner())->match(event, session));
    event.user_id = ADMIN_USER_ID;
    REQUIRE_FALSE(to_cond(owner())->match(event, session));
    event.user_id = MEMBER_USER_ID;
    REQUIRE_FALSE(to_cond(owner())->match(event, session));
}
