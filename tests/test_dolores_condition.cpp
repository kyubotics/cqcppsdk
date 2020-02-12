#include "catch.hpp"
#include "dolores/condition.hpp"
#include "test_dolores.hpp"

using namespace dolores;
using namespace dolores::cond;

std::pair<cq::PrivateMessageEvent, StrAnyMap> construct() {
    return {cq::PrivateMessageEvent(1, 1, "hello, world", 0, cq::PrivateMessageEvent::SubType::FRIEND), StrAnyMap()};
}

template <typename Cond>
std::shared_ptr<Condition> to_cond(Cond &&c) {
    return std::make_shared<std::decay_t<Cond>>(std::forward<Cond>(c));
}

TEST_CASE("cond::Not", "[condition]") {
    auto [event, state] = construct();
    REQUIRE(!(*to_cond(Not(contains("ll"))))(event, state));
    REQUIRE((*to_cond(Not(contains("foo"))))(event, state));
}

TEST_CASE("cond::And", "[condition]") {
    auto [event, state] = construct();
    REQUIRE((*to_cond(And(contains("ll"), contains("wor"))))(event, state));
    REQUIRE((*to_cond((contains("ll") & contains("wor"))))(event, state));
    REQUIRE(!(*to_cond((contains("foo") & contains("wor"))))(event, state));
}

TEST_CASE("cond::Or", "[condition]") {
    auto [event, state] = construct();
    REQUIRE((*to_cond(And(contains("ll"), contains("wor"))))(event, state));
    REQUIRE((*to_cond((contains("ll") & contains("wor"))))(event, state));
    REQUIRE(!(*to_cond((contains("foo") & contains("wor"))))(event, state));
}

TEST_CASE("cond::All", "[condition]") {
    auto [event, state] = construct();
    REQUIRE((*to_cond(All(contains("ll"), contains("wor"), contains(","))))(event, state));
    REQUIRE(!(*to_cond(All(contains("ll"), contains("wor"), contains("foo"))))(event, state));
}

TEST_CASE("cond::type", "[condition]") {
    auto [event, state] = construct();
    const auto &ev_ref = event;
    const auto &ev2_ref = static_cast<const cq::MessageEvent &>(ev_ref);
    REQUIRE((*to_cond(type<cq::PrivateMessageEvent>))(ev_ref, state));
    REQUIRE((*to_cond(type<cq::PrivateMessageEvent>))(ev2_ref, state));
    REQUIRE(!(*to_cond(type<cq::GroupMessageEvent>))(ev2_ref, state));
}

TEST_CASE("cond::unblocked", "[condition]") {
    auto [event, state] = construct();
    REQUIRE((*to_cond(unblocked()))(event, state));
    event.block();
    REQUIRE(!(*to_cond(unblocked()))(event, state));
}

TEST_CASE("cond::startswith", "[condition]") {
    auto [event, state] = construct();
    REQUIRE((*to_cond(startswith("hello")))(event, state));
    REQUIRE(!(*to_cond(startswith("foo")))(event, state));
}

TEST_CASE("cond::endswith", "[condition]") {
    auto [event, state] = construct();
    REQUIRE((*to_cond(endswith("world")))(event, state));
    REQUIRE(!(*to_cond(startswith("foo")))(event, state));
}

TEST_CASE("cond::contains", "[condition]") {
    auto [event, state] = construct();
    REQUIRE((*to_cond(contains("world")))(event, state));
    REQUIRE((*to_cond(contains(",")))(event, state));
    REQUIRE(!(*to_cond(contains("foo")))(event, state));
}

TEST_CASE("cond::command", "[condition]") {
    auto [event, state] = construct();
    event.message = "/echo";
    REQUIRE((*to_cond(command("echo")))(event, state));
    REQUIRE(state.count(command::STARTER) > 0);
    REQUIRE(state.count(command::NAME) > 0);
    REQUIRE(state.count(command::ARGUMENT) > 0);
    REQUIRE(std::any_cast<std::string>(state.at(command::NAME)) == "echo");

    event.message = "/echo   hello";
    REQUIRE((*to_cond(command("echo")))(event, state));
    REQUIRE(std::any_cast<std::string_view>(state.at(command::ARGUMENT)) == "  hello");

    event.message = "/e";
    REQUIRE(!(*to_cond(command("echo")))(event, state));
    event.message = "";
    REQUIRE(!(*to_cond(command("echo")))(event, state));
}

TEST_CASE("cond::user", "[condition]") {
    auto [event, state] = construct();
    REQUIRE((*to_cond(user()))(event, state));
    REQUIRE((*to_cond(user({event.user_id})))(event, state));
    REQUIRE(!(*to_cond(user({event.user_id + 1})))(event, state));
    REQUIRE((*to_cond(user::exclude({event.user_id + 1})))(event, state));
    REQUIRE(!(*to_cond(user::exclude({event.user_id})))(event, state));

    auto ge = cq::GroupAdminEvent(1, 1, cq::GroupAdminEvent::SubType::SET);
    REQUIRE((*to_cond(user()))(ge, state));
}

TEST_CASE("cond::direct", "[condition]") {
    auto [event, state] = construct();
    REQUIRE((*to_cond(direct()))(event, state));
    REQUIRE((*to_cond(direct({event.user_id})))(event, state));
    REQUIRE(!(*to_cond(direct({event.user_id + 1})))(event, state));
    REQUIRE((*to_cond(direct::exclude({event.user_id + 1})))(event, state));
    REQUIRE(!(*to_cond(direct::exclude({event.user_id})))(event, state));

    auto ge = cq::GroupAdminEvent(1, 1, cq::GroupAdminEvent::SubType::SET);
    REQUIRE(!(*to_cond(direct()))(ge, state));
}

TEST_CASE("cond::group", "[condition]") {
    auto [_, state] = construct();
    auto event = cq::GroupAdminEvent(1, 1, cq::GroupAdminEvent::SubType::SET);
    REQUIRE((*to_cond(group()))(event, state));
    REQUIRE((*to_cond(group({event.group_id})))(event, state));
    REQUIRE(!(*to_cond(group({event.group_id + 1})))(event, state));
    REQUIRE((*to_cond(group::exclude({event.group_id + 1})))(event, state));
    REQUIRE(!(*to_cond(group::exclude({event.group_id})))(event, state));

    REQUIRE(!(*to_cond(group()))(construct().first, state));
}

TEST_CASE("cond::discuss", "[condition]") {
    auto [_, state] = construct();
    auto event = cq::DiscussMessageEvent(1, 1, "hello", 0, 1);
    REQUIRE((*to_cond(discuss()))(event, state));

    REQUIRE(!(*to_cond(discuss()))(construct().first, state));
}

TEST_CASE("cond::group_roles", "[condition]") {
    auto [_, state] = construct();
    auto event = cq::GroupMessageEvent(MEMBER_USER_ID, 1, "hello", 0, 1, cq::Anonymous());
    REQUIRE((*to_cond(group_roles({cq::GroupRole::MEMBER})))(event, state));
    REQUIRE(!(*to_cond(group_roles({cq::GroupRole::OWNER})))(event, state));

    REQUIRE((*to_cond(group_roles({cq::GroupRole::OWNER})))(construct().first, state));
}

TEST_CASE("cond::admin", "[condition]") {
    auto [_, state] = construct();
    auto event = cq::GroupMessageEvent(ADMIN_USER_ID, 1, "hello", 0, 1, cq::Anonymous());
    REQUIRE((*to_cond(admin()))(event, state));
    event.user_id = OWNER_USER_ID;
    REQUIRE((*to_cond(admin()))(event, state));
    event.user_id = MEMBER_USER_ID;
    REQUIRE(!(*to_cond(admin()))(event, state));
}

TEST_CASE("cond::owner", "[condition]") {
    auto [_, state] = construct();
    auto event = cq::GroupMessageEvent(OWNER_USER_ID, 1, "hello", 0, 1, cq::Anonymous());
    REQUIRE((*to_cond(owner()))(event, state));
    event.user_id = ADMIN_USER_ID;
    REQUIRE(!(*to_cond(owner()))(event, state));
    event.user_id = MEMBER_USER_ID;
    REQUIRE(!(*to_cond(owner()))(event, state));
}
