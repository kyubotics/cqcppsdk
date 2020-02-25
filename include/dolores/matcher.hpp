#pragma once

#include <cqcppsdk/cqcppsdk.h>

#include <algorithm>
#include <functional>
#include <iostream>
#include <string>
#include <string_view>
#include <type_traits>
#include <unordered_set>
#include <utility>
#include <vector>

#include "anymap.hpp"
#include "string.hpp"
#include "traits.hpp"
#include "watashi.hpp"

namespace dolores {
    class MatcherBase {
    public:
        virtual bool match(const cq::MessageEvent &event, StrAnyMap &matcher_data) const {
            return match(static_cast<const cq::UserEvent &>(event), matcher_data);
        }

        virtual bool match(const cq::NoticeEvent &event, StrAnyMap &matcher_data) const {
            return match(static_cast<const cq::UserEvent &>(event), matcher_data);
        }

        virtual bool match(const cq::RequestEvent &event, StrAnyMap &matcher_data) const {
            return match(static_cast<const cq::UserEvent &>(event), matcher_data);
        }

        virtual bool match(const cq::UserEvent &event, StrAnyMap &matcher_data) const {
            return false;
        }
    };

    class MessageMatcher : virtual public MatcherBase {
    public:
        virtual bool match(const cq::Target &target, const std::string_view &message, StrAnyMap &matcher_data) const {
            return false;
        }

        bool match(const cq::MessageEvent &event, StrAnyMap &matcher_data) const final {
            return match(event.target, event.message, matcher_data);
        }
    };

    namespace matchers {
        class _NotMatcher : public MatcherBase {
        public:
            template <typename T, typename = std::enable_if_t<is_matcher_v<T>>>
            explicit _NotMatcher(T &&matcher) : _matcher(std::make_shared<std::decay_t<T>>(std::forward<T>(matcher))) {
            }

            bool match(const cq::MessageEvent &event, StrAnyMap &matcher_data) const override {
                return _match(event, matcher_data);
            }

            bool match(const cq::NoticeEvent &event, StrAnyMap &matcher_data) const override {
                return _match(event, matcher_data);
            }

            bool match(const cq::RequestEvent &event, StrAnyMap &matcher_data) const override {
                return _match(event, matcher_data);
            }

            bool match(const cq::UserEvent &event, StrAnyMap &matcher_data) const override {
                return _match(event, matcher_data);
            }

        protected:
            std::shared_ptr<MatcherBase> _matcher;

            template <typename E>
            bool _match(const E &event, StrAnyMap &matcher_data) const {
                return !_matcher->match(event, matcher_data);
            }
        };

        class _NotMessageMatcher : public MessageMatcher {
        public:
            template <typename T, typename = std::enable_if_t<is_message_matcher_v<T>>>
            explicit _NotMessageMatcher(T &&matcher)
                : _matcher(std::make_shared<std::decay_t<T>>(std::forward<T>(matcher))) {
            }

            bool match(const cq::Target &target, const std::string_view &message,
                       StrAnyMap &matcher_data) const override {
                return !_matcher->match(target, message, matcher_data);
            }

        protected:
            std::shared_ptr<MessageMatcher> _matcher;
        };

        class _AndMatcher : public MatcherBase {
        public:
            template <typename TL, typename TR, typename = std::enable_if_t<is_matcher_v<TL, TR>>>
            _AndMatcher(TL &&lhs, TR &&rhs)
                : _lhs(std::make_shared<std::decay_t<TL>>(std::forward<TL>(lhs))),
                  _rhs(std::make_shared<std::decay_t<TR>>(std::forward<TR>(rhs))) {
            }

            bool match(const cq::MessageEvent &event, StrAnyMap &matcher_data) const override {
                return _match(event, matcher_data);
            }

            bool match(const cq::NoticeEvent &event, StrAnyMap &matcher_data) const override {
                return _match(event, matcher_data);
            }

            bool match(const cq::RequestEvent &event, StrAnyMap &matcher_data) const override {
                return _match(event, matcher_data);
            }

            bool match(const cq::UserEvent &event, StrAnyMap &matcher_data) const override {
                return _match(event, matcher_data);
            }

        protected:
            std::shared_ptr<MatcherBase> _lhs;
            std::shared_ptr<MatcherBase> _rhs;

            template <typename E>
            bool _match(const E &event, StrAnyMap &matcher_data) const {
                return _lhs->match(event, matcher_data) && _rhs->match(event, matcher_data);
            }
        };

        class _AndMessageMatcher : public MessageMatcher {
        public:
            template <typename TL, typename TR, typename = std::enable_if_t<is_message_matcher_v<TL, TR>>>
            _AndMessageMatcher(TL &&lhs, TR &&rhs)
                : _lhs(std::make_shared<std::decay_t<TL>>(std::forward<TL>(lhs))),
                  _rhs(std::make_shared<std::decay_t<TR>>(std::forward<TR>(rhs))) {
            }

            bool match(const cq::Target &target, const std::string_view &message,
                       StrAnyMap &matcher_data) const override {
                return _lhs->match(target, message, matcher_data) && _rhs->match(target, message, matcher_data);
            }

        protected:
            std::shared_ptr<MessageMatcher> _lhs;
            std::shared_ptr<MessageMatcher> _rhs;
        };

        class _OrMatcher : public MatcherBase {
        public:
            template <typename TL, typename TR, typename = std::enable_if_t<is_matcher_v<TL, TR>>>
            _OrMatcher(TL &&lhs, TR &&rhs)
                : _lhs(std::make_shared<std::decay_t<TL>>(std::forward<TL>(lhs))),
                  _rhs(std::make_shared<std::decay_t<TR>>(std::forward<TR>(rhs))) {
            }

            bool match(const cq::MessageEvent &event, StrAnyMap &matcher_data) const override {
                return _match(event, matcher_data);
            }

            bool match(const cq::NoticeEvent &event, StrAnyMap &matcher_data) const override {
                return _match(event, matcher_data);
            }

            bool match(const cq::RequestEvent &event, StrAnyMap &matcher_data) const override {
                return _match(event, matcher_data);
            }

            bool match(const cq::UserEvent &event, StrAnyMap &matcher_data) const override {
                return _match(event, matcher_data);
            }

        protected:
            std::shared_ptr<MatcherBase> _lhs;
            std::shared_ptr<MatcherBase> _rhs;

            template <typename E>
            bool _match(const E &event, StrAnyMap &matcher_data) const {
                return _lhs->match(event, matcher_data) || _rhs->match(event, matcher_data);
            }
        };

        class _OrMessageMatcher : public MessageMatcher {
        public:
            template <typename TL, typename TR, typename = std::enable_if_t<is_message_matcher_v<TL, TR>>>
            _OrMessageMatcher(TL &&lhs, TR &&rhs)
                : _lhs(std::make_shared<std::decay_t<TL>>(std::forward<TL>(lhs))),
                  _rhs(std::make_shared<std::decay_t<TR>>(std::forward<TR>(rhs))) {
            }

            bool match(const cq::Target &target, const std::string_view &message,
                       StrAnyMap &matcher_data) const override {
                return _lhs->match(target, message, matcher_data) || _rhs->match(target, message, matcher_data);
            }

        protected:
            std::shared_ptr<MessageMatcher> _lhs;
            std::shared_ptr<MessageMatcher> _rhs;
        };

        template <typename T, typename = std::enable_if_t<is_matcher_v<T>>>
        inline auto operator!(T &&matcher) {
            if constexpr (is_message_matcher_v<T>) {
                return _NotMessageMatcher(std::forward<T>(matcher));
            } else {
                return _NotMatcher(std::forward<T>(matcher));
            }
        }

        template <typename TL, typename TR, typename = std::enable_if_t<is_matcher_v<TL, TR>>>
        inline auto operator&&(TL &&lhs, TR &&rhs) {
            if constexpr (is_message_matcher_v<TL> && is_message_matcher_v<TR>) {
                return _AndMessageMatcher(std::forward<TL>(lhs), std::forward<TR>(rhs));
            } else {
                return _AndMatcher(std::forward<TL>(lhs), std::forward<TR>(rhs));
            }
        }

        template <typename TL, typename TR, typename = std::enable_if_t<is_matcher_v<TL, TR>>>
        inline auto operator||(TL &&lhs, TR &&rhs) {
            if constexpr (is_message_matcher_v<TL> && is_message_matcher_v<TR>) {
                return _OrMessageMatcher(std::forward<TL>(lhs), std::forward<TR>(rhs));
            } else {
                return _OrMatcher(std::forward<TL>(lhs), std::forward<TR>(rhs));
            }
        }

        class all : public MatcherBase {
        public:
            template <typename... Matchers>
            explicit all(Matchers &&... matchers)
                : _matchers({std::make_shared<std::decay_t<Matchers>>(std::forward<Matchers>(matchers))...}) {
            }

            bool match(const cq::MessageEvent &event, StrAnyMap &matcher_data) const override {
                return _match(event, matcher_data);
            }

            bool match(const cq::NoticeEvent &event, StrAnyMap &matcher_data) const override {
                return _match(event, matcher_data);
            }

            bool match(const cq::RequestEvent &event, StrAnyMap &matcher_data) const override {
                return _match(event, matcher_data);
            }

            bool match(const cq::UserEvent &event, StrAnyMap &matcher_data) const override {
                return _match(event, matcher_data);
            }

        protected:
            std::vector<std::shared_ptr<MatcherBase>> _matchers;

            template <typename E>
            bool _match(const E &event, StrAnyMap &matcher_data) const {
                return std::all_of(_matchers.cbegin(), _matchers.cend(), [&](const auto &matcher) {
                    return matcher->match(event, matcher_data);
                });
            }
        };

        template <typename E>
        struct _type {
            class matcher_t : public MatcherBase {
            public:
                bool match(const cq::UserEvent &event, StrAnyMap &matcher_data) const override {
                    return typeid(event) == typeid(E);
                }
            };

            static constexpr matcher_t matcher{};
        };

        template <typename E>
        constexpr auto type = _type<E>::matcher;

        class unblocked : public MatcherBase {
        public:
            bool match(const cq::UserEvent &event, StrAnyMap &matcher_data) const override {
                return !event.blocked();
            }
        };

        class startswith : public MessageMatcher {
        public:
            explicit startswith(std::string prefix) : _prefix(std::move(prefix)) {
            }

            bool match(const cq::Target &target, const std::string_view &message,
                       StrAnyMap &matcher_data) const override {
                return string::startswith(message, _prefix);
            }

        protected:
            std::string _prefix;
        };

        class endswith : public MessageMatcher {
        public:
            explicit endswith(std::string suffix) : _suffix(std::move(suffix)) {
            }

            bool match(const cq::Target &target, const std::string_view &message,
                       StrAnyMap &matcher_data) const override {
                return string::endswith(message, _suffix);
            }

        protected:
            std::string _suffix;
        };

        class contains : public MessageMatcher {
        public:
            explicit contains(std::string sub) : _sub(std::move(sub)) {
            }

            bool match(const cq::Target &target, const std::string_view &message,
                       StrAnyMap &matcher_data) const override {
                return string::contains(message, _sub);
            }

        protected:
            std::string _sub;
        };

        class command : public MessageMatcher {
        public:
            static constexpr auto STARTER = "_cond__command__starter";
            static constexpr auto NAME = "_cond__command__name";
            static constexpr auto ARGUMENT = "_cond__command__argument";

            explicit command(std::string name, std::unordered_set<std::string> starters = {})
                : command({std::move(name)}, std::move(starters)) {
            }

            command(std::initializer_list<std::string> names, std::unordered_set<std::string> starters = {})
                : _names(names), _starters(std::move(starters)) {
            }

            bool match(const cq::Target &target, const std::string_view &message,
                       StrAnyMap &matcher_data) const override {
                static const std::unordered_set<std::string> default_starters = {"/", "!", ".", "！", "。"};

                const auto message_v = string::string_view_from(
                    std::find_if_not(message.cbegin(), message.cend(), cq::utils::isspace_s), message.cend());

                bool starter_ok = false;
                std::string_view matched_starter_v;
                for (const auto &starter : (_starters.empty() ? default_starters : _starters)) {
                    if (string::startswith(message_v, starter)) {
                        starter_ok = true;
                        matched_starter_v = std::string_view(message_v.data(), starter.length());
                        break;
                    }
                }
                if (!starter_ok) return false;

                const auto beg = message_v.cbegin() + matched_starter_v.length();
                const auto end = message_v.cend();
                const auto first_space = std::find_if(beg, end, cq::utils::isspace_s);
                const auto candidate_name_v = string::string_view_from(beg, first_space);
                const auto res = std::find(_names.cbegin(), _names.cend(), candidate_name_v) != _names.cend();

                if (res) {
                    matcher_data[STARTER] = matched_starter_v;
                    matcher_data[NAME] = candidate_name_v;

                    if (first_space < message_v.cend()) {
                        matcher_data[ARGUMENT] = string::string_view_from(first_space + 1, message_v.cend());
                    } else {
                        matcher_data[ARGUMENT] = std::string_view("");
                    }
                }

                return res;
            }

        protected:
            std::unordered_set<std::string> _names;
            std::unordered_set<std::string> _starters;
        };

        class to_me : public MessageMatcher {
        public:
            to_me() = default;

            template <typename T, typename = std::enable_if_t<std::is_base_of_v<MessageMatcher, T>>>
            explicit to_me(T &&matcher) : _sub_matcher(std::make_shared<std::decay_t<T>>(std::forward<T>(matcher))) {
            }

            bool match(const cq::Target &target, const std::string_view &message,
                       StrAnyMap &matcher_data) const override {
                if (target.is_private()) {
                    return _sub_matcher ? _sub_matcher->match(target, message, matcher_data) : true;
                }

                using cq::message::MessageSegment;
                const auto at_me_seg = cq::to_string(MessageSegment::at(watashi::user_id()));
                const auto at_me_off = message.find(at_me_seg);
                if (at_me_off == std::string_view::npos) {
                    return false;
                } else if (!_sub_matcher) {
                    return true;
                }

                // assert: _sub_matcher is not null

                const auto is_full_of_spaces = [](const auto &s) {
                    return std::find_if_not(s.cbegin(), s.cend(), cq::utils::isspace_s) == s.cend();
                };

                const auto before_at_v = std::string_view(message.data(), at_me_off);
                const auto after_at_v =
                    string::string_view_from(message.cbegin() + at_me_off + at_me_seg.length(), message.cend());
                std::string_view cut_message_v;
                if (is_full_of_spaces(before_at_v)) {
                    // @me is at the beginning of message
                    cut_message_v = after_at_v;
                } else if (is_full_of_spaces(after_at_v)) {
                    // @me is at the end of message
                    cut_message_v = before_at_v;
                } else {
                    // @me is in the middle of message
                    cut_message_v = message;
                }
                return _sub_matcher->match(target, cut_message_v, matcher_data);
            }

        private:
            std::shared_ptr<MessageMatcher> _sub_matcher;
        };

        class user : public MatcherBase {
        public:
            user() = default;

            explicit user(std::unordered_set<int64_t> include) : _include_users(std::move(include)) {
            }

            static user exclude(std::unordered_set<int64_t> exclude) {
                user u;
                u._exclude_users = std::move(exclude);
                return u;
            }

            bool match(const cq::UserEvent &event, StrAnyMap &matcher_data) const override {
                if (!_include_users.empty()) {
                    return std::find(_include_users.cbegin(), _include_users.cend(), event.user_id)
                           != _include_users.cend();
                }
                if (!_exclude_users.empty()) {
                    return std::find(_exclude_users.cbegin(), _exclude_users.cend(), event.user_id)
                           == _exclude_users.cend();
                }
                return true; // both include_users & exclude_users are empty
            }

        protected:
            std::unordered_set<int64_t> _include_users;
            std::unordered_set<int64_t> _exclude_users;
        };

        class direct : public user {
        public:
            using user::user;

            static direct exclude(std::unordered_set<int64_t> exclude) {
                direct d;
                d._exclude_users = std::move(exclude);
                return d;
            }

            bool match(const cq::UserEvent &event, StrAnyMap &matcher_data) const override {
                if (!event.target.is_private()) return false;
                return user::match(event, matcher_data);
            }
        };

        class group : public MatcherBase {
        public:
            group() = default;

            explicit group(std::unordered_set<int64_t> include) : _include_groups(std::move(include)) {
            }

            static group exclude(std::unordered_set<int64_t> exclude) {
                group g;
                g._exclude_groups = std::move(exclude);
                return g;
            }

            bool match(const cq::UserEvent &event, StrAnyMap &matcher_data) const override {
                if (!event.target.is_group()) return false;

                const auto group_id = event.target.group_id.value_or(0);
                if (!_include_groups.empty()) {
                    return std::find(_include_groups.cbegin(), _include_groups.cend(), group_id)
                           != _include_groups.cend();
                }
                if (!_exclude_groups.empty()) {
                    return std::find(_exclude_groups.cbegin(), _exclude_groups.cend(), group_id)
                           == _exclude_groups.cend();
                }
                return true; // both include_groups & exclude_groups are empty
            }

        protected:
            std::unordered_set<int64_t> _include_groups;
            std::unordered_set<int64_t> _exclude_groups;
        };

        class discuss : public MatcherBase {
        public:
            bool match(const cq::UserEvent &event, StrAnyMap &matcher_data) const override {
                return event.target.is_discuss();
            }
        };

        class group_roles : public MatcherBase {
        public:
            explicit group_roles(std::unordered_set<cq::GroupRole> roles) : _roles(std::move(roles)) {
            }

            bool match(const cq::UserEvent &event, StrAnyMap &matcher_data) const override {
                if (!event.target.is_group()) return true; // ignore non-group event

                const auto group_id = event.target.group_id.value_or(0);
                try {
                    const auto mi = cq::get_group_member_info(group_id, event.user_id);
                    return std::find(_roles.cbegin(), _roles.cend(), mi.role) != _roles.cend();
                } catch (cq::ApiError &) {
                    // try again with cache disabled
                    try {
                        const auto mi = cq::get_group_member_info(group_id, event.user_id, true);
                        return std::find(_roles.cbegin(), _roles.cend(), mi.role) != _roles.cend();
                    } catch (cq::ApiError &) {
                        return false;
                    }
                }
            }

        protected:
            std::unordered_set<cq::GroupRole> _roles;
        };

        class admin : public group_roles {
        public:
            admin() : group_roles({cq::GroupRole::ADMIN, cq::GroupRole::OWNER}) {
            }
        };

        class owner : public group_roles {
        public:
            owner() : group_roles({cq::GroupRole::OWNER}) {
            }
        };
    } // namespace matchers
} // namespace dolores
