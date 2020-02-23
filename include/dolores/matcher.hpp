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
#include <bitset>

#include "session.hpp"
#include "string.hpp"
#include "traits.hpp"
#include "watashi.hpp"

namespace dolores {
    namespace matchers {
        template <typename T, typename = std::enable_if_t<is_matcher_v<std::decay_t<T>>>>
        constexpr auto operator!(T &&matcher) {
            return [matcher](auto &&... args) { return !matcher(std::forward<decltype(args)>(args)...); };
        }

        template <typename TL, typename TR,
                  typename = std::enable_if_t<is_matcher_v<std::decay_t<TL>> && is_matcher_v<std::decay_t<TR>>>>
        constexpr auto operator&&(TL &&lhs, TR &&rhs) {
            return [lhs, rhs](auto &&... args) { return lhs(args...) && rhs(args...); };
        }

        template <typename TL, typename TR,
                  typename = std::enable_if_t<is_matcher_v<std::decay_t<TL>> && is_matcher_v<std::decay_t<TR>>>>
        constexpr auto operator||(TL &&lhs, TR &&rhs) {
            return [lhs, rhs](auto &&... args) { return lhs(args...) || rhs(args...); };
        }

        template <typename... Matchers>
        constexpr auto all(Matchers &&... matchers) {
            return [=](auto &&... args) { return (matchers(args...) && ...); };
        }

        template <typename E>
        constexpr auto type = [](const auto &event, Session &session) { return typeid(event) == typeid(E); };

        constexpr auto unblocked() {
            return [](const cq::UserEvent &event, Session &session) { return !event.blocked(); };
        }

        class startswith {
        public:
            explicit constexpr startswith(std::string_view prefix) : _prefix(prefix) {
            }

            constexpr bool operator()(const cq::Target &target, const std::string_view &message, Session &session) const {
                return string::startswith(message, _prefix);
            }

            bool operator()(const cq::MessageEvent &event, Session &session) const {
                return (*this)(event.target, event.message, session);
            }

        protected:
            std::string_view _prefix;
        };

        class endswith {
        public:
            explicit constexpr endswith(std::string_view suffix) : _suffix(suffix) {
            }

            constexpr bool operator()(const cq::Target &target, const std::string_view &message, Session &session) const {
                return string::endswith(message, _suffix);
            }

            bool operator()(const cq::MessageEvent &event, Session &session) const {
                return (*this)(event.target, event.message, session);
            }

        protected:
            std::string_view _suffix;
        };

        class contains {
        public:
            explicit constexpr contains(std::string_view sub) : _sub(sub) {
            }

            constexpr bool operator()(const cq::Target &target, const std::string_view &message, Session &session) const {
                return string::contains(message, _sub);
            }

            bool operator()(const cq::MessageEvent &event, Session &session) const {
                return (*this)(event.target, event.message, session);
            }

        protected:
            std::string_view _sub;
        };

        class command {
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

            bool operator()(const cq::Target &target, const std::string_view &message, Session &session) const {
                static const std::vector<std::string> default_starters = {"/", "!", ".", "！", "。"};
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
                    session[STARTER] = matched_starter_v;
                    session[NAME] = candidate_name_v;

                    if (first_space < message_v.cend()) {
                        session[ARGUMENT] = string::string_view_from(first_space + 1, message_v.cend());
                    } else {
                        session[ARGUMENT] = std::string_view("");
                    }
                }

                return res;
            }

            bool operator()(const cq::MessageEvent &event, Session &session) const {
                return (*this)(event.target, event.message, session);
            }

        protected:
            std::vector<std::string> _names;
            std::vector<std::string> _starters;
        };

        template <class T = void>
        class to_me {
        public:
            explicit to_me(T &&matcher) : _sub_matcher(matcher) {
            }

            bool operator()(const cq::Target &target, const std::string_view &message, Session &session) const {
                if (target.is_private()) {
                    return _sub_matcher(target, message, session);
                }

                using cq::message::MessageSegment;
                const auto at_me_seg = cq::to_string(MessageSegment::at(watashi::user_id()));
                const auto at_me_off = message.find(at_me_seg);
                if (at_me_off == std::string_view::npos) {
                    return false;
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
                return _sub_matcher(target, cut_message_v, session);
            };

            bool operator()(const cq::MessageEvent &event, Session &session) const {
                return (*this)(event.target, event.message, session);
            }

        private:
            T _sub_matcher;
        };

        template <>
        class to_me<void> {
        public:
            constexpr to_me() = default;

            bool operator()(const cq::Target &target, const std::string_view &message, Session &session) const {
                if (target.is_private()) {
                    return true;
                }

                using cq::message::MessageSegment;
                const auto at_me_seg = cq::to_string(MessageSegment::at(watashi::user_id()));
                const auto at_me_off = message.find(at_me_seg);
                if (at_me_off == std::string_view::npos) {
                    return false;
                }

                return true;
            };

            bool operator()(const cq::MessageEvent &event, Session &session) const {
                return (*this)(event.target, event.message, session);
            }
        };
        to_me()->to_me<void>;
        template <class T>
        to_me(T)->to_me<T>;

        class user {
        public:
            user() = default;

            explicit user(std::vector<int64_t> include) : _include_users(std::move(include)) {
            }

            static user exclude(std::vector<int64_t> exclude) {
                user u;
                u._exclude_users = std::move(exclude);
                return u;
            }

            bool operator()(const cq::UserEvent &event, Session &session) const {
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

            bool operator()(const cq::UserEvent &event, Session &session) const {
                if (!event.target.is_private()) return false;
                return user::operator()(event, session);
            }
        };

        class group {
        public:
            group() = default;

            explicit group(std::vector<int64_t> include) : _include_groups(std::move(include)) {
            }

            static group exclude(std::vector<int64_t> exclude) {
                group g;
                g._exclude_groups = std::move(exclude);
                return g;
            }

            bool operator()(const cq::UserEvent &event, Session &session) const {
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

        class discuss {
        public:
            bool operator()(const cq::UserEvent &event, Session &session) const {
                return event.target.is_discuss();
            }
        };

        class group_roles {
        public:
            static constexpr std::size_t MAX_ROLES = 3;

            //template<std::size_t...I>
            //constexpr group_roles(std::initializer_list<cq::GroupRole> il, std::index_sequence<I...>) : _roles(((1ull << static_cast<std::size_t>(*(il.begin() + I))) || ...)) {}

            //constexpr group_roles(std::initializer_list<cq::GroupRole> il) : group_roles(il, std::make_index_sequence<MAX_ROLES>()) {}

            template<class...Args, class = std::enable_if_t<(std::is_same_v<Args, cq::GroupRole> && ...)>>
            constexpr group_roles(Args...args) : _roles(((1ull << static_cast<std::size_t>(args)) || ...)) {  }

            bool operator()(const cq::UserEvent &event, Session &session) const {
                if (!event.target.is_group()) return true; // ignore non-group event

                const auto group_id = event.target.group_id.value_or(0);
                try {
                    const auto mi = cq::get_group_member_info(group_id, event.user_id);
                    return _roles.test(static_cast<std::size_t>(mi.role));
                } catch (cq::ApiError &) {
                    // try again with cache disabled
                    try {
                        const auto mi = cq::get_group_member_info(group_id, event.user_id, true);
                        return _roles.test(static_cast<std::size_t>(mi.role));
                    } catch (cq::ApiError &) {
                        return false;
                    }
                }
            }

        protected:
            std::bitset<MAX_ROLES> _roles;
        };

        class admin : public group_roles {
        public:
            constexpr admin() : group_roles({cq::GroupRole::ADMIN, cq::GroupRole::OWNER}) {
            }
        };

        class owner : public group_roles {
        public:
            constexpr owner() : group_roles({cq::GroupRole::OWNER}) {
            }
        };
    } // namespace matchers
} // namespace dolores
