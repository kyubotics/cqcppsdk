#include "cqcppsdk/cqcppsdk.h"
#include "dolores/matcher.hpp"
#include "dolores/traits.hpp"

#include "catch.hpp"

TEST_CASE("is_derived_from_user_event_v", "[traits]") {
    using namespace dolores;

    REQUIRE(is_derived_from_user_event_v<cq::MessageEvent>);
    REQUIRE(is_derived_from_user_event_v<cq::NoticeEvent>);
    REQUIRE(is_derived_from_user_event_v<cq::RequestEvent>);
    REQUIRE(is_derived_from_user_event_v<cq::PrivateMessageEvent>);
    REQUIRE_FALSE(is_derived_from_user_event_v<cq::UserEvent>);
    REQUIRE_FALSE(is_derived_from_user_event_v<cq::Event>);

    struct FakeEvent {};
    REQUIRE_FALSE(is_derived_from_user_event_v<FakeEvent>);
}

TEST_CASE("is_matcher_v", "[traits]") {
    using namespace dolores;
    using namespace dolores::matchers;

    REQUIRE(is_matcher_v<MatcherBase>);
    REQUIRE(is_matcher_v<MessageMatcher>);
    REQUIRE(is_matcher_v<startswith>);
    REQUIRE(is_matcher_v<group>);
    REQUIRE(is_matcher_v<decltype(!user({10001000}))>);
    REQUIRE(is_matcher_v<decltype(startswith("prefix") && command("hello"))>);
    REQUIRE(is_matcher_v<decltype(startswith("prefix") || direct())>);

    struct DiyMatcher : MatcherBase {};
    REQUIRE(is_matcher_v<DiyMatcher>);

    REQUIRE(is_matcher_v<DiyMatcher, group_roles>);

    struct FakeMatcher {};
    REQUIRE_FALSE(is_matcher_v<FakeMatcher>);
    REQUIRE_FALSE(is_matcher_v<to_me, FakeMatcher>);
    REQUIRE_FALSE(is_matcher_v<FakeMatcher, to_me>);
}

TEST_CASE("is_message_matcher_v", "[traits]") {
    using namespace dolores;
    using namespace dolores::matchers;

    REQUIRE(is_message_matcher_v<MessageMatcher>);
    REQUIRE(is_message_matcher_v<startswith>);
    REQUIRE(is_message_matcher_v<endswith>);
    REQUIRE(is_message_matcher_v<contains>);
    REQUIRE(is_message_matcher_v<command>);
    REQUIRE(is_message_matcher_v<to_me>);

    REQUIRE(is_message_matcher_v<decltype(!startswith("prefix"))>);
    REQUIRE(is_message_matcher_v<decltype(startswith("prefix") && command("hello"))>);
    REQUIRE(is_message_matcher_v<decltype(startswith("prefix") || command("hello"))>);

    REQUIRE_FALSE(is_message_matcher_v<MatcherBase>);
    REQUIRE_FALSE(is_message_matcher_v<group>);
    REQUIRE_FALSE(is_message_matcher_v<decltype(user({10001000}) && startswith("prefix"))>);

    struct DiyMatcher : MatcherBase {};
    REQUIRE_FALSE(is_message_matcher_v<DiyMatcher>);

    struct DiyMessageMatcher : MessageMatcher {};
    REQUIRE(is_message_matcher_v<DiyMessageMatcher>);
}
