#include "dolores/matcher.hpp"
#include "test_dolores.hpp"

#include "catch.hpp"

using namespace dolores;
using namespace dolores::matchers;

std::pair<cq::PrivateMessageEvent, Session> construct_pm() {
    return {cq::PrivateMessageEvent(1, 1, "hello, world", 0, cq::PrivateMessageEvent::SubType::FRIEND), Session()};
}

template <typename Cond>
std::shared_ptr<MatcherBase> to_matcher(Cond &&c) {
    return std::make_shared<std::decay_t<Cond>>(std::forward<Cond>(c));
}

TEST_CASE("matchers::operator!", "[matcher]") {
    auto [event, session] = construct_pm();
    REQUIRE_FALSE(to_matcher(!contains("ll"))->match(event, session));
    REQUIRE(to_matcher(!contains("foo"))->match(event, session));
}

TEST_CASE("matchers::operator&&", "[matcher]") {
    auto [event, session] = construct_pm();
    REQUIRE(to_matcher((contains("ll") && contains("wor")))->match(event, session));
    REQUIRE_FALSE(to_matcher((contains("foo") && contains("wor")))->match(event, session));
}

TEST_CASE("matchers::operator||", "[matcher]") {
    auto [event, session] = construct_pm();
    REQUIRE(to_matcher((contains("ll") || contains("foo")))->match(event, session));
    REQUIRE_FALSE(to_matcher((contains("foo") || contains("bar")))->match(event, session));
}

TEST_CASE("matchers::all", "[matcher]") {
    auto [event, session] = construct_pm();
    REQUIRE(to_matcher(all(contains("ll"), contains("wor"), contains(",")))->match(event, session));
    REQUIRE_FALSE(to_matcher(all(contains("ll"), contains("wor"), contains("foo")))->match(event, session));
}

TEST_CASE("matchers::type", "[matcher]") {
    auto [event, session] = construct_pm();
    const auto &ev_ref = event;
    const auto &ev2_ref = static_cast<const cq::MessageEvent &>(ev_ref);
    REQUIRE(to_matcher(type<cq::PrivateMessageEvent>)->match(ev_ref, session));
    REQUIRE(to_matcher(type<cq::PrivateMessageEvent>)->match(ev2_ref, session));
    REQUIRE_FALSE(to_matcher(type<cq::GroupMessageEvent>)->match(ev2_ref, session));
}

TEST_CASE("matchers::unblocked", "[matcher]") {
    auto [event, session] = construct_pm();
    REQUIRE(to_matcher(unblocked())->match(event, session));
    event.block();
    REQUIRE_FALSE(to_matcher(unblocked())->match(event, session));
}

TEST_CASE("matchers::startswith", "[matcher]") {
    auto [event, session] = construct_pm();
    REQUIRE(to_matcher(startswith("hello"))->match(event, session));
    REQUIRE_FALSE(to_matcher(startswith("foo"))->match(event, session));
}

TEST_CASE("matchers::endswith", "[matcher]") {
    auto [event, session] = construct_pm();
    REQUIRE(to_matcher(endswith("world"))->match(event, session));
    REQUIRE_FALSE(to_matcher(startswith("foo"))->match(event, session));
}

TEST_CASE("matchers::contains", "[matcher]") {
    auto [event, session] = construct_pm();
    REQUIRE(to_matcher(contains("world"))->match(event, session));
    REQUIRE(to_matcher(contains(","))->match(event, session));
    REQUIRE_FALSE(to_matcher(contains("foo"))->match(event, session));
}

TEST_CASE("matchers::command", "[matcher]") {
    SECTION("without argument") {
        auto [event, session] = construct_pm();
        event.message = "/echo";
        REQUIRE(to_matcher(command("echo"))->match(event, session));
        REQUIRE(session.count(command::STARTER) > 0);
        REQUIRE(session.count(command::NAME) > 0);
        REQUIRE(session.count(command::ARGUMENT) > 0);
        REQUIRE(session.get<std::string_view>(command::NAME) == "echo");
    }

    SECTION("with argument") {
        auto [event, session] = construct_pm();
        event.message = "/echo   hello";
        REQUIRE(to_matcher(command({"echo", "e"}))->match(event, session));
        REQUIRE(session.get<std::string_view>(command::ARGUMENT) == "  hello");
    }

    SECTION("empty starter") {
        auto [event, session] = construct_pm();
        event.message = "echo wow";
        REQUIRE(to_matcher(command("echo", {"", "?"}))->match(event, session));
    }

    SECTION("spaces before command") {
        auto [event, session] = construct_pm();
        event.message = "    /echo wow";
        REQUIRE(to_matcher(command("echo"))->match(event, session));
    }

    SECTION("bad command name") {
        auto [event, session] = construct_pm();
        event.message = "/e";
        REQUIRE_FALSE(to_matcher(command("echo"))->match(event, session));
    }

    SECTION("empty message") {
        auto [event, session] = construct_pm();
        event.message = "";
        REQUIRE_FALSE(to_matcher(command("echo"))->match(event, session));
    }

    SECTION("message full of spaces") {
        auto [event, session] = construct_pm();
        event.message = "    ";
        REQUIRE_FALSE(to_matcher(command("echo"))->match(event, session));
    }
}

TEST_CASE("matchers::at_me", "[matcher]") {
    watashi::_user_id() = LOGIN_ID;

    SECTION("private message") {
        auto [event, session] = construct_pm();
        REQUIRE(to_matcher(to_me())->match(event, session));
        event.message = "/echo";
        REQUIRE(to_matcher(to_me(command("echo")))->match(event, session));
        REQUIRE(session.count(command::NAME) > 0);
    }

    SECTION("group message") {
        using cq::message::MessageSegment;
        auto event = cq::GroupMessageEvent(MEMBER_USER_ID, 1, "/echo", 0, 1, cq::Anonymous());
        Session session;
        REQUIRE_FALSE(to_matcher(to_me())->match(event, session));

        SECTION("@ at the beginning") {
            session.clear();
            event.message = "  " + MessageSegment::at(LOGIN_ID) + "   /echo";
            REQUIRE(to_matcher(to_me())->match(event, session));
            REQUIRE(to_matcher(to_me(command("echo")))->match(event, session));
            REQUIRE(session.get<std::string_view>(command::NAME) == "echo");
        }

        SECTION("@ in the middle") {
            session.clear();
            event.message = "/echo " + MessageSegment::at(LOGIN_ID) + " wow";
            REQUIRE(to_matcher(to_me(command("echo")))->match(event, session));
            REQUIRE(session.get<std::string_view>(command::NAME) == "echo");
        }

        SECTION("@ at the end") {
            session.clear();
            event.message = "/echo   " + MessageSegment::at(LOGIN_ID) + "   ";
            REQUIRE(to_matcher(to_me(command("echo")))->match(event, session));
            REQUIRE(session.get<std::string_view>(command::NAME) == "echo");
        }
    }
}

TEST_CASE("matchers::user", "[matcher]") {
    auto [event, session] = construct_pm();
    REQUIRE(to_matcher(user())->match(event, session));
    REQUIRE(to_matcher(user({event.user_id}))->match(event, session));
    REQUIRE_FALSE(to_matcher(user({event.user_id + 1}))->match(event, session));
    REQUIRE(to_matcher(user::exclude({event.user_id + 1}))->match(event, session));
    REQUIRE_FALSE(to_matcher(user::exclude({event.user_id}))->match(event, session));

    auto ge = cq::GroupAdminEvent(1, 1, cq::GroupAdminEvent::SubType::SET);
    REQUIRE(to_matcher(user())->match(ge, session));
}

TEST_CASE("matchers::direct", "[matcher]") {
    auto [event, session] = construct_pm();
    REQUIRE(to_matcher(direct())->match(event, session));
    REQUIRE(to_matcher(direct({event.user_id}))->match(event, session));
    REQUIRE_FALSE(to_matcher(direct({event.user_id + 1}))->match(event, session));
    REQUIRE(to_matcher(direct::exclude({event.user_id + 1}))->match(event, session));
    REQUIRE_FALSE(to_matcher(direct::exclude({event.user_id}))->match(event, session));

    auto ge = cq::GroupAdminEvent(1, 1, cq::GroupAdminEvent::SubType::SET);
    REQUIRE_FALSE(to_matcher(direct())->match(ge, session));
}

TEST_CASE("matchers::group", "[matcher]") {
    auto [_, session] = construct_pm();
    auto event = cq::GroupAdminEvent(1, 1, cq::GroupAdminEvent::SubType::SET);
    REQUIRE(to_matcher(group())->match(event, session));
    REQUIRE(to_matcher(group({event.group_id}))->match(event, session));
    REQUIRE_FALSE(to_matcher(group({event.group_id + 1}))->match(event, session));
    REQUIRE(to_matcher(group::exclude({event.group_id + 1}))->match(event, session));
    REQUIRE_FALSE(to_matcher(group::exclude({event.group_id}))->match(event, session));

    REQUIRE_FALSE(to_matcher(group())->match(construct_pm().first, session));
}

TEST_CASE("matchers::discuss", "[matcher]") {
    auto [_, session] = construct_pm();
    auto event = cq::DiscussMessageEvent(1, 1, "hello", 0, 1);
    REQUIRE(to_matcher(discuss())->match(event, session));

    REQUIRE_FALSE(to_matcher(discuss())->match(construct_pm().first, session));
}

TEST_CASE("matchers::group_roles", "[matcher]") {
    auto [_, session] = construct_pm();
    auto event = cq::GroupMessageEvent(MEMBER_USER_ID, 1, "hello", 0, 1, cq::Anonymous());
    REQUIRE(to_matcher(group_roles({cq::GroupRole::MEMBER}))->match(event, session));
    REQUIRE_FALSE(to_matcher(group_roles({cq::GroupRole::OWNER}))->match(event, session));

    REQUIRE(to_matcher(group_roles({cq::GroupRole::OWNER}))->match(construct_pm().first, session));
}

TEST_CASE("matchers::admin", "[matcher]") {
    auto [_, session] = construct_pm();
    auto event = cq::GroupMessageEvent(ADMIN_USER_ID, 1, "hello", 0, 1, cq::Anonymous());
    REQUIRE(to_matcher(admin())->match(event, session));
    event.user_id = OWNER_USER_ID;
    REQUIRE(to_matcher(admin())->match(event, session));
    event.user_id = MEMBER_USER_ID;
    REQUIRE_FALSE(to_matcher(admin())->match(event, session));
}

TEST_CASE("matchers::owner", "[matcher]") {
    auto [_, session] = construct_pm();
    auto event = cq::GroupMessageEvent(OWNER_USER_ID, 1, "hello", 0, 1, cq::Anonymous());
    REQUIRE(to_matcher(owner())->match(event, session));
    event.user_id = ADMIN_USER_ID;
    REQUIRE_FALSE(to_matcher(owner())->match(event, session));
    event.user_id = MEMBER_USER_ID;
    REQUIRE_FALSE(to_matcher(owner())->match(event, session));
}
