#pragma once

#include <cqcppsdk/cqcppsdk.h>

#include <algorithm>
#include <functional>
#include <iostream>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include "string.hpp"
#include "traits.hpp"

namespace dolores {
    struct Condition {
        virtual bool operator()(const cq::MessageEvent &event) const {
            return operator()(static_cast<const cq::UserEvent &>(event));
        }

        virtual bool operator()(const cq::NoticeEvent &event) const {
            return operator()(static_cast<const cq::UserEvent &>(event));
        }

        virtual bool operator()(const cq::RequestEvent &event) const {
            return operator()(static_cast<const cq::UserEvent &>(event));
        }

        virtual bool operator()(const cq::UserEvent &event) const {
            return false;
        }
    };

    namespace cond {
        struct AndExpr : Condition {
            std::shared_ptr<Condition> lhs;
            std::shared_ptr<Condition> rhs;

            AndExpr(std::shared_ptr<Condition> lhs, std::shared_ptr<Condition> rhs)
                : lhs(std::move(lhs)), rhs(std::move(rhs)) {
            }

            template <typename E>
            bool __call__(const E &event) const {
                return (*lhs)(event) && (*rhs)(event);
            }

            bool operator()(const cq::MessageEvent &event) const override {
                return __call__(event);
            }

            bool operator()(const cq::NoticeEvent &event) const override {
                return __call__(event);
            }

            bool operator()(const cq::RequestEvent &event) const override {
                return __call__(event);
            }

            bool operator()(const cq::UserEvent &event) const override {
                return __call__(event);
            }
        };

        struct OrExpr : Condition {
            std::shared_ptr<Condition> lhs;
            std::shared_ptr<Condition> rhs;

            OrExpr(std::shared_ptr<Condition> lhs, std::shared_ptr<Condition> rhs)
                : lhs(std::move(lhs)), rhs(std::move(rhs)) {
            }

            template <typename E>
            bool __call__(const E &event) const {
                return (*lhs)(event) || (*rhs)(event);
            }

            bool operator()(const cq::MessageEvent &event) const override {
                return __call__(event);
            }

            bool operator()(const cq::NoticeEvent &event) const override {
                return __call__(event);
            }

            bool operator()(const cq::RequestEvent &event) const override {
                return __call__(event);
            }

            bool operator()(const cq::UserEvent &event) const override {
                return __call__(event);
            }
        };

        template <typename TL, typename TR, typename = enable_if_is_condition_t<TL, TR>>
        inline AndExpr operator&(TL &&lhs, TR &&rhs) {
            return AndExpr(std::make_shared<std::decay_t<TL>>(std::forward<TL>(lhs)),
                           std::make_shared<std::decay_t<TR>>(std::forward<TR>(rhs)));
        }

        template <typename TL, typename TR, typename = enable_if_is_condition_t<TL, TR>>
        inline OrExpr operator|(TL &&lhs, TR &&rhs) {
            return OrExpr(std::make_shared<std::decay_t<TL>>(std::forward<TL>(lhs)),
                          std::make_shared<std::decay_t<TR>>(std::forward<TR>(rhs)));
        }

        struct All : Condition {
            std::vector<std::shared_ptr<Condition>> conditions;

            template <typename... Args>
            explicit All(Args &&... args)
                : conditions({std::make_shared<std::decay_t<Args>>(std::forward<Args>(args))...}) {
            }

            template <typename E>
            bool __call__(const E &event) const {
                return std::all_of(
                    conditions.cbegin(), conditions.cend(), [&](const auto &cond) { return (*cond)(event); });
            }

            bool operator()(const cq::MessageEvent &event) const override {
                return __call__(event);
            }

            bool operator()(const cq::NoticeEvent &event) const override {
                return __call__(event);
            }

            bool operator()(const cq::RequestEvent &event) const override {
                return __call__(event);
            }

            bool operator()(const cq::UserEvent &event) const override {
                return __call__(event);
            }
        };

        template <typename E>
        struct _type {
            struct condition_t : Condition {
                bool operator()(const cq::UserEvent &event) const override {
                    return typeid(event) == typeid(E);
                }
            };

            static constexpr condition_t condition{};
        };

        template <typename E>
        constexpr auto type = _type<E>::condition;

        struct unblocked : Condition {
            bool operator()(const cq::UserEvent &event) const override {
                return !event.blocked();
            }
        };

        struct startswith : Condition {
            std::string prefix;

            explicit startswith(std::string prefix) : prefix(std::move(prefix)) {
            }

            bool operator()(const cq::MessageEvent &event) const override {
                return string::startswith(event.message, prefix);
            }
        };

        struct endswith : Condition {
            std::string suffix;

            explicit endswith(std::string suffix) : suffix(std::move(suffix)) {
            }

            bool operator()(const cq::MessageEvent &event) const override {
                return string::endswith(event.message, suffix);
            }
        };

        struct contains : Condition {
            std::string sub;

            explicit contains(std::string sub) : sub(std::move(sub)) {
            }

            bool operator()(const cq::MessageEvent &event) const override {
                return string::contains(event.message, sub);
            }
        };

        struct command : Condition {
            std::vector<std::string> names;
            std::vector<std::string> starters;
            std::vector<std::string> default_starters = {"/", "!", ".", "！", "。"};

            explicit command(std::string name, std::vector<std::string> starters = {})
                : command({std::move(name)}, std::move(starters)) {
            }

            command(std::initializer_list<std::string> names, std::vector<std::string> starters = {})
                : names(names), starters(std::move(starters)) {
            }

            bool operator()(const cq::MessageEvent &event) const override {
                bool starter_ok = false;
                std::string matched_starter;
                for (const auto &starter : (starters.empty() ? default_starters : starters)) {
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
                return std::find(names.cbegin(), names.cend(), std::string(beg, first_space)) != names.cend();
            }
        };

        struct user : Condition {
            std::vector<int64_t> include_users;
            std::vector<int64_t> exclude_users;

            user() = default;

            explicit user(std::vector<int64_t> include) : include_users(std::move(include)) {
            }

            static user exclude(std::vector<int64_t> exclude) {
                user u;
                u.exclude_users = std::move(exclude);
                return u;
            }

            bool operator()(const cq::UserEvent &event) const override {
                if (!include_users.empty()) {
                    return std::find(include_users.cbegin(), include_users.cend(), event.user_id)
                           != include_users.cend();
                }
                if (!exclude_users.empty()) {
                    return std::find(exclude_users.cbegin(), exclude_users.cend(), event.user_id)
                           == exclude_users.cend();
                }
                return true; // both include_users & exclude_users are empty
            }
        };

        struct direct : user {
            using user::user;

            static direct exclude(std::vector<int64_t> exclude) {
                direct d;
                d.exclude_users = std::move(exclude);
                return d;
            }

            bool operator()(const cq::UserEvent &event) const override {
                if (!event.target.is_private()) return false;
                return user::operator()(event);
            }
        };

        struct group : Condition {
            std::vector<int64_t> include_groups;
            std::vector<int64_t> exclude_groups;

            group() = default;

            explicit group(std::vector<int64_t> include) : include_groups(std::move(include)) {
            }

            static group exclude(std::vector<int64_t> exclude) {
                group g;
                g.exclude_groups = std::move(exclude);
                return g;
            }

            bool operator()(const cq::UserEvent &event) const override {
                if (!event.target.is_group()) return false;

                const auto group_id = event.target.group_id.value_or(0);
                if (!include_groups.empty()) {
                    return std::find(include_groups.cbegin(), include_groups.cend(), group_id) != include_groups.cend();
                }
                if (!exclude_groups.empty()) {
                    return std::find(exclude_groups.cbegin(), exclude_groups.cend(), group_id) == exclude_groups.cend();
                }
                return true; // both include_groups & exclude_groups are empty
            }
        };

        struct discuss : Condition {
            bool operator()(const cq::UserEvent &event) const override {
                return event.target.is_discuss();
            }
        };

        struct group_roles : Condition {
            std::vector<cq::GroupRole> roles;

            explicit group_roles(std::vector<cq::GroupRole> roles) : roles(std::move(roles)) {
            }

            bool operator()(const cq::UserEvent &event) const override {
                if (!event.target.is_group()) return true; // ignore non-group event

                const auto group_id = event.target.group_id.value_or(0);
                try {
                    const auto mi = cq::get_group_member_info(group_id, event.user_id);
                    return std::find(roles.cbegin(), roles.cend(), mi.role) != roles.cend();
                } catch (cq::ApiError &) {
                    // try again with cache disabled
                    try {
                        const auto mi = cq::get_group_member_info(group_id, event.user_id, true);
                        return std::find(roles.cbegin(), roles.cend(), mi.role) != roles.cend();
                    } catch (cq::ApiError &) {
                        return false;
                    }
                }
            }
        };

        struct admin : group_roles {
            admin() : group_roles({cq::GroupRole::ADMIN, cq::GroupRole::OWNER}) {
            }
        };

        struct owner : group_roles {
            owner() : group_roles({cq::GroupRole::OWNER}) {
            }
        };
    } // namespace cond
} // namespace dolores
