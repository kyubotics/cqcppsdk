#pragma once

#include "common.hpp"

#include "exception.hpp"

#include "../utils/base64.hpp"
#include "../utils/binpack.hpp"

namespace cq {
    class ObjectHelper {
    public:
        // 从 Base64 字符串解析数据对象,
        // 注意, 与 T::from_bytes 不同, 后者从二进制数据中提取对象
        template <typename T>
        static T from_base64(const std::string &b64) noexcept(false) {
            return T::from_bytes(utils::base64_decode(b64));
        }

        // 从 Base64 字符串解析数据对象集合
        template <typename Container>
        static Container multi_from_base64(const std::string &b64) noexcept(false) {
            Container result;
            auto inserter = std::back_inserter(result);
            auto pack = utils::BinPack(utils::base64_decode(b64));
            try {
                const auto count = pack.pop_int<int32_t>();
                for (auto i = 0; i < count; i++) {
                    *inserter = Container::value_type::from_bytes(pack.pop_token());
                }
            } catch (BytesNotEnough &) {
                throw ParseError("failed to parse from bytes to multiple objects");
            }
            return result;
        }
    };

    // 性别
    enum class Sex {
        MALE = 0, // 男
        FEMALE = 1, // 女
        UNKNOWN = 255, // 未知
    };

    // 群成员角色
    enum class GroupRole {
        MEMBER = 1, // 普通成员
        ADMIN = 2, // 管理员
        OWNER = 3, // 群主
    };

    // 用户信息
    struct User {
        int64_t user_id = 0; // 用户 ID (QQ 号)
        std::string nickname; // 昵称
        Sex sex = Sex::UNKNOWN; // 性别
        int32_t age = 0; // 年龄

    private:
        const static size_t MIN_SIZE = 18;

        friend class ObjectHelper;

        static User from_bytes(std::string &&bytes) noexcept(false) {
            auto pack = utils::BinPack(bytes);
            User stranger;
            try {
                stranger.user_id = pack.pop_int<int64_t>();
                stranger.nickname = pack.pop_string();
                stranger.sex = static_cast<Sex>(pack.pop_int<int32_t>());
                stranger.age = pack.pop_int<int32_t>();
            } catch (BytesNotEnough &) {
                throw ParseError("failed to parse from bytes to a User object");
            }
            return stranger;
        }
    };

    // 好友信息
    struct Friend : User {
        // int64_t user_id; // 继承自 User 类
        // std::string nickname; // 继承自 User 类
        std::string remark; // 备注

    private:
        const static size_t MIN_SIZE = 12;

        friend class ObjectHelper;

        static Friend from_bytes(std::string &&bytes) noexcept(false) {
            auto pack = utils::BinPack(bytes);
            Friend frnd;
            try {
                frnd.user_id = pack.pop_int<int64_t>();
                frnd.nickname = pack.pop_string();
                frnd.remark = pack.pop_string();
            } catch (BytesNotEnough &) {
                throw ParseError("failed to parse from bytes to a Friend object");
            }
            return frnd;
        }

    private:
        Sex sex; // 屏蔽 User 类的 sex 属性
        int32_t age; // 屏蔽 User 类的 age 属性
    };

    // 群信息
    struct Group {
        int64_t group_id = 0; // 群号
        std::string group_name; // 群名
        int32_t member_count = 0; // 成员数, 仅 get_group_info() 返回
        int32_t max_member_count = 0; // 最大成员数(容量), 仅 get_group_info() 返回

    private:
        const static size_t MIN_SIZE = 10;

        friend class ObjectHelper;

        static Group from_bytes(std::string &&bytes) noexcept(false) {
            auto pack = utils::BinPack(bytes);
            Group group;
            try {
                group.group_id = pack.pop_int<int64_t>();
                group.group_name = pack.pop_string();
                try {
                    // 尝试获取 member_count 和 max_member_count,
                    // 如果正在处理的是 get_group_list() 的返回结果, 会失败, 将忽略异常继续
                    group.member_count = pack.pop_int<int32_t>();
                    group.max_member_count = pack.pop_int<int32_t>();
                } catch (BytesNotEnough &) {
                }
            } catch (BytesNotEnough &) {
                throw ParseError("failed to parse from bytes to a Group object");
            }
            return group;
        }
    };

    // 群成员信息
    struct GroupMember : User {
        int64_t group_id = 0; // 群号
        // int64_t user_id; // 继承自 User 类
        // std::string nickname; // 继承自 User 类
        std::string card; // 名片(现在为群昵称)
        // Sex sex; // 继承自 User 类
        // int32_t age; // 继承自 User 类
        std::string area; // 地区
        int32_t join_time = 0; // 入群时间
        int32_t last_sent_time = 0; // 最近发言时间
        std::string level; // 等级
        GroupRole role = GroupRole::MEMBER; // 角色(权限)
        bool unfriendly = false; // 是否有不良记录
        std::string title; // 头衔
        int32_t title_expire_time = 0; // 头衔过期时间
        bool card_changeable = false; // 是否可修改名片

    private:
        const static size_t MIN_SIZE = 58;

        friend class ObjectHelper;

        static GroupMember from_bytes(std::string &&bytes) noexcept(false) {
            auto pack = utils::BinPack(bytes);
            GroupMember member;
            try {
                member.group_id = pack.pop_int<int64_t>();
                member.user_id = pack.pop_int<int64_t>();
                member.nickname = pack.pop_string();
                member.card = pack.pop_string();
                member.sex = static_cast<Sex>(pack.pop_int<int32_t>());
                member.age = pack.pop_int<int32_t>();
                member.area = pack.pop_string();
                member.join_time = pack.pop_int<int32_t>();
                member.last_sent_time = pack.pop_int<int32_t>();
                member.level = pack.pop_string();
                member.role = static_cast<GroupRole>(pack.pop_int<int32_t>());
                member.unfriendly = pack.pop_bool();
                member.title = pack.pop_string();
                member.title_expire_time = pack.pop_int<int32_t>();
                member.card_changeable = pack.pop_bool();
            } catch (BytesNotEnough &) {
                throw ParseError("failed to parse from bytes to a GroupMember object");
            }
            return member;
        }
    };

    // 匿名信息
    struct Anonymous {
        int64_t id = 0; // ID, 具体含义不明
        std::string name; // 匿名昵称
        std::string token; // 一串二进制数据, 具体含义不明
        std::string base64; // 整个 Anonymous 对象的 Base64 编码字符串

    private:
        const static size_t MIN_SIZE = 12;

        friend class ObjectHelper;

        static Anonymous from_bytes(std::string &&bytes) noexcept(false) {
            auto pack = utils::BinPack(bytes);
            Anonymous anonymous;
            try {
                anonymous.id = pack.pop_int<int64_t>();
                anonymous.name = pack.pop_string();
                anonymous.token = pack.pop_token();
                // 注意: 这里不给 base64 属性赋值, 而是在下面特化的 ObjectHelper::from_base64 函数中赋值
            } catch (BytesNotEnough &) {
                throw ParseError("failed to parse from bytes to an Anonymous object");
            }
            return anonymous;
        }
    };

    template <>
    inline Anonymous ObjectHelper::from_base64<Anonymous>(const std::string &b64) {
        auto anonymous = Anonymous::from_bytes(utils::base64_decode(b64));
        anonymous.base64 = b64;
        return anonymous;
    }

    // 文件信息
    struct File {
        std::string id; // ID
        std::string name; // 名称
        int64_t size = 0; // 大小(字节)
        int64_t busid = 0; // 某种 ID, 具体含义不明

    private:
        const static size_t MIN_SIZE = 20;

        friend class ObjectHelper;

        static File from_bytes(std::string &&bytes) noexcept(false) {
            auto pack = utils::BinPack(bytes);
            File file;
            try {
                file.id = pack.pop_string();
                file.name = pack.pop_string();
                file.size = pack.pop_int<int64_t>();
                file.busid = pack.pop_int<int64_t>();
            } catch (BytesNotEnough &) {
                throw ParseError("failed to parse from bytes to a File object");
            }
            return file;
        }
    };
} // namespace cq
