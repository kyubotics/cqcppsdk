#pragma once

#include <cqcppsdk/cqcppsdk.h>

#include <algorithm>
#include <functional>
#include <iostream>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

#include "session.hpp"
#include "string.hpp"
#include "traits.hpp"

namespace dolores {
    class MatcherBase {
    public:
        virtual bool match(const cq::MessageEvent &event) const {
            return match(static_cast<const cq::UserEvent &>(event));
        }

        virtual bool match(const cq::NoticeEvent &event) const {
            return match(static_cast<const cq::UserEvent &>(event));
        }

        virtual bool match(const cq::RequestEvent &event) const {
            return match(static_cast<const cq::UserEvent &>(event));
        }

        virtual bool match(const cq::UserEvent &event) const {
            return false;
        }

        virtual bool match(const cq::MessageEvent &event, Session &session) const {
            return match(event) || match(static_cast<const cq::UserEvent &>(event), session);
        }

        virtual bool match(const cq::NoticeEvent &event, Session &session) const {
            return match(event) || match(static_cast<const cq::UserEvent &>(event), session);
        }

        virtual bool match(const cq::RequestEvent &event, Session &session) const {
            return match(event) || match(static_cast<const cq::UserEvent &>(event), session);
        }

        virtual bool match(const cq::UserEvent &event, Session &session) const {
            return match(event);
        }
    };

    namespace matchers {
        class Not : public MatcherBase {
        public:
            template <typename T, typename = enable_if_is_matcher_t<T>>
            explicit Not(T &&matcher) : _matcher(std::make_shared<std::decay_t<T>>(std::forward<T>(matcher))) {
            }

            bool match(const cq::MessageEvent &event, Session &session) const override {
                return _match(event, session);
            }

            bool match(const cq::NoticeEvent &event, Session &session) const override {
                return _match(event, session);
            }

            bool match(const cq::RequestEvent &event, Session &session) const override {
                return _match(event, session);
            }

            bool match(const cq::UserEvent &event, Session &session) const override {
                return _match(event, session);
            }

        protected:
            std::shared_ptr<MatcherBase> _matcher;

            template <typename E>
            bool _match(const E &event, Session &session) const {
                return !_matcher->match(event, session);
            }
        };

        class And : public MatcherBase {
        public:
            template <typename TL, typename TR, typename = enable_if_is_matcher_t<TL, TR>>
            And(TL &&lhs, TR &&rhs)
                : _lhs(std::make_shared<std::decay_t<TL>>(std::forward<TL>(lhs))),
                  _rhs(std::make_shared<std::decay_t<TR>>(std::forward<TR>(rhs))) {
            }

            bool match(const cq::MessageEvent &event, Session &session) const override {
                return _match(event, session);
            }

            bool match(const cq::NoticeEvent &event, Session &session) const override {
                return _match(event, session);
            }

            bool match(const cq::RequestEvent &event, Session &session) const override {
                return _match(event, session);
            }

            bool match(const cq::UserEvent &event, Session &session) const override {
                return _match(event, session);
            }

        protected:
            std::shared_ptr<MatcherBase> _lhs;
            std::shared_ptr<MatcherBase> _rhs;

            template <typename E>
            bool _match(const E &event, Session &session) const {
                return _lhs->match(event, session) && _rhs->match(event, session);
            }
        };

        class Or : public MatcherBase {
        public:
            template <typename TL, typename TR, typename = enable_if_is_matcher_t<TL, TR>>
            Or(TL &&lhs, TR &&rhs)
                : _lhs(std::make_shared<std::decay_t<TL>>(std::forward<TL>(lhs))),
                  _rhs(std::make_shared<std::decay_t<TR>>(std::forward<TR>(rhs))) {
            }

            bool match(const cq::MessageEvent &event, Session &session) const override {
                return _match(event, session);
            }

            bool match(const cq::NoticeEvent &event, Session &session) const override {
                return _match(event, session);
            }

            bool match(const cq::RequestEvent &event, Session &session) const override {
                return _match(event, session);
            }

            bool match(const cq::UserEvent &event, Session &session) const override {
                return _match(event, session);
            }

        protected:
            std::shared_ptr<MatcherBase> _lhs;
            std::shared_ptr<MatcherBase> _rhs;

            template <typename E>
            bool _match(const E &event, Session &session) const {
                return _lhs->match(event, session) || _rhs->match(event, session);
            }
        };

        template <typename T, typename = enable_if_is_matcher_t<T>>
        inline Not operator!(T &&matcher) {
            return Not(std::forward<T>(matcher));
        }

        template <typename TL, typename TR, typename = enable_if_is_matcher_t<TL, TR>>
        inline And operator&&(TL &&lhs, TR &&rhs) {
            return And(std::forward<TL>(lhs), std::forward<TR>(rhs));
        }

        template <typename TL, typename TR, typename = enable_if_is_matcher_t<TL, TR>>
        inline Or operator||(TL &&lhs, TR &&rhs) {
            return Or(std::forward<TL>(lhs), std::forward<TR>(rhs));
        }

        class All : public MatcherBase {
        public:
            template <typename... Matchers>
            explicit All(Matchers &&... matchers)
                : _matchers({std::make_shared<std::decay_t<Matchers>>(std::forward<Matchers>(matchers))...}) {
            }

            bool match(const cq::MessageEvent &event, Session &session) const override {
                return _match(event, session);
            }

            bool match(const cq::NoticeEvent &event, Session &session) const override {
                return _match(event, session);
            }

            bool match(const cq::RequestEvent &event, Session &session) const override {
                return _match(event, session);
            }

            bool match(const cq::UserEvent &event, Session &session) const override {
                return _match(event, session);
            }

        protected:
            std::vector<std::shared_ptr<MatcherBase>> _matchers;

            template <typename E>
            bool _match(const E &event, Session &session) const {
                return std::all_of(_matchers.cbegin(), _matchers.cend(), [&](const auto &matcher) {
                    return matcher->match(event, session);
                });
            }
        };

        template <typename E>
        struct _type {
            class matcher_t : public MatcherBase {
            public:
                bool match(const cq::UserEvent &event) const override {
                    return typeid(event) == typeid(E);
                }
            };

            static constexpr matcher_t matcher{};
        };

        template <typename E>
        constexpr auto type = _type<E>::matcher;

        class unblocked : public MatcherBase {
        public:
            bool match(const cq::UserEvent &event) const override {
                return !event.blocked();
            }
        };

        class startswith : public MatcherBase {
        public:
            explicit startswith(std::string prefix) : _prefix(std::move(prefix)) {
            }

            bool match(const cq::MessageEvent &event) const override {
                return string::startswith(event.message, _prefix);
            }

        protected:
            std::string _prefix;
        };

        class endswith : public MatcherBase {
        public:
            explicit endswith(std::string suffix) : _suffix(std::move(suffix)) {
            }

            bool match(const cq::MessageEvent &event) const override {
                return string::endswith(event.message, _suffix);
            }

        protected:
            std::string _suffix;
        };

        class contains : public MatcherBase {
        public:
            explicit contains(std::string sub) : _sub(std::move(sub)) {
            }

            bool match(const cq::MessageEvent &event) const override {
                return string::contains(event.message, _sub);
            }

        protected:
            std::string _sub;
        };

        class command : public MatcherBase {
        public:
            static constexpr auto STARTER = "_cond__command__starter";
            static constexpr auto NAME = "_cond__command__name";
            static constexpr auto ARGUMENT = "_cond__command__argument";

            explicit command(std::string name, std::vector<std::string> starters = {})
                : command({std::move(name)}, std::move(starters)) {
            }

            command(std::initializer_list<std::string> names, std::vector<std::string> starters = {})
                : _names(names), _starters(std::move(starters)) {
            }

            bool match(const cq::MessageEvent &event, Session &session) const override {
                static const std::vector<std::string> default_starters = {"/", "!", ".", "！", "。"};

                bool starter_ok = false;
                std::string matched_starter;
                for (const auto &starter : (_starters.empty() ? default_starters : _starters)) {
                    if (string::startswith(event.message, starter)) {
                        starter_ok = true;
                        matched_starter = starter;
                        break;
                    }
                }
                if (!starter_ok) return false;

                const auto beg = event.message.cbegin() + matched_starter.length();
                const auto end = event.message.cend();
                const auto first_space = std::find_if(beg, end, cq::utils::isspace_s);
                const auto candidate_name = std::string(beg, first_space);
                const auto res = std::find(_names.cbegin(), _names.cend(), candidate_name) != _names.cend();

                if (res) {
                    session[STARTER] = matched_starter;
                    session[NAME] = candidate_name;

                    const auto arg_beg_off = static_cast<size_t>(first_space - event.message.cbegin() + 1);
                    if (arg_beg_off < event.message.length()) {
                        session[ARGUMENT] =
                            std::string_view(event.message.data() + arg_beg_off, event.message.length() - arg_beg_off);
                    } else {
                        session[ARGUMENT] = std::string_view("");
                    }
                }

                return res;
            }

        protected:
            std::vector<std::string> _names;
            std::vector<std::string> _starters;
        };

        class user : public MatcherBase {
        public:
            user() = default;

            explicit user(std::vector<int64_t> include) : _include_users(std::move(include)) {
            }

            static user exclude(std::vector<int64_t> exclude) {
                user u;
                u._exclude_users = std::move(exclude);
                return u;
            }

            bool match(const cq::UserEvent &event) const override {
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
            std::vector<int64_t> _include_users;
            std::vector<int64_t> _exclude_users;
        };

        class direct : public user {
        public:
            using user::user;

            static direct exclude(std::vector<int64_t> exclude) {
                direct d;
                d._exclude_users = std::move(exclude);
                return d;
            }

            bool match(const cq::UserEvent &event) const override {
                if (!event.target.is_private()) return false;
                return user::match(event);
            }
        };

        class group : public MatcherBase {
        public:
            group() = default;

            explicit group(std::vector<int64_t> include) : _include_groups(std::move(include)) {
            }

            static group exclude(std::vector<int64_t> exclude) {
                group g;
                g._exclude_groups = std::move(exclude);
                return g;
            }

            bool match(const cq::UserEvent &event) const override {
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
            std::vector<int64_t> _include_groups;
            std::vector<int64_t> _exclude_groups;
        };

        class discuss : public MatcherBase {
            bool match(const cq::UserEvent &event) const override {
                return event.target.is_discuss();
            }
        };

        class group_roles : public MatcherBase {
        public:
            explicit group_roles(std::vector<cq::GroupRole> roles) : _roles(std::move(roles)) {
            }

            bool match(const cq::UserEvent &event) const override {
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
            std::vector<cq::GroupRole> _roles;
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
