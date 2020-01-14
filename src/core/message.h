#pragma once

#include "./common.h"

#include "./api.h"

namespace cq::message {
    // 对字符串做 CQ 码转义
    std::string escape(const std::string &str, const bool escape_comma = true);

    // 对字符串做 CQ 码去转义
    std::string unescape(const std::string &str);

    // 消息段 (即 CQ 码)
    struct MessageSegment {
        std::string type; // 消息段类型 (即 CQ 码的功能名)
        std::map<std::string, std::string> data; // 消息段数据 (即 CQ 码参数), 字符串全部使用未经 CQ 码转义的原始文本

        // 转换为字符串形式
        operator std::string() const {
            std::string s;
            if (this->type.empty()) {
                return s;
            }
            if (this->type == "text") {
                if (const auto it = this->data.find("text"); it != this->data.end()) {
                    s += escape((*it).second, false);
                }
            } else {
                s += "[CQ:" + this->type;
                for (const auto &item : this->data) {
                    s += "," + item.first + "=" + escape(item.second, true);
                }
                s += "]";
            }
            return s;
        }

        // 纯文本
        static MessageSegment text(const std::string &text) {
            return {"text", {{"text", text}}};
        }

        // Emoji 表情
        static MessageSegment emoji(const uint32_t id) {
            return {"emoji", {{"id", std::to_string(id)}}};
        }

        // QQ 表情
        static MessageSegment face(const int id) {
            return {"face", {{"id", std::to_string(id)}}};
        }

        // 图片
        static MessageSegment image(const std::string &file) {
            return {"image", {{"file", file}}};
        }

        // 语音
        static MessageSegment record(const std::string &file, const bool magic = false) {
            return {"record", {{"file", file}, {"magic", std::to_string(magic)}}};
        }

        // @某人
        static MessageSegment at(const int64_t user_id) {
            return {"at", {{"qq", std::to_string(user_id)}}};
        }

        // 猜拳魔法表情
        static MessageSegment rps() {
            return {"rps", {}};
        }

        // 掷骰子魔法表情
        static MessageSegment dice() {
            return {"dice", {}};
        }

        // 戳一戳
        static MessageSegment shake() {
            return {"shake", {}};
        }

        // 匿名发消息
        static MessageSegment anonymous(const bool ignore_failure = false) {
            return {"anonymous", {{"ignore", std::to_string(ignore_failure)}}};
        }

        // 链接分享
        static MessageSegment share(const std::string &url, const std::string &title, const std::string &content = "",
                                    const std::string &image_url = "") {
            return {"share", {{"url", url}, {"title", title}, {"content", content}, {"image", image_url}}};
        }

        enum class ContactType { USER, GROUP };

        // 推荐好友, 推荐群
        static MessageSegment contact(const ContactType &type, const int64_t id) {
            return {
                "contact",
                {
                    {"type", type == ContactType::USER ? "qq" : "group"},
                    {"id", std::to_string(id)},
                },
            };
        }

        // 位置
        static MessageSegment location(const double latitude, const double longitude, const std::string &title = "",
                                       const std::string &content = "") {
            return {
                "location",
                {
                    {"lat", std::to_string(latitude)},
                    {"lon", std::to_string(longitude)},
                    {"title", title},
                    {"content", content},
                },
            };
        }

        // 音乐
        static MessageSegment music(const std::string &type, const int64_t id) {
            return {"music", {{"type", type}, {"id", std::to_string(id)}}};
        }

        // 音乐
        static MessageSegment music(const std::string &type, const int64_t id, const int32_t style) {
            return {"music", {{"type", type}, {"id", std::to_string(id)}, {"style", std::to_string(style)}}};
        }

        // 音乐自定义分享
        static MessageSegment music(const std::string &url, const std::string &audio_url, const std::string &title,
                                    const std::string &content = "", const std::string &image_url = "") {
            return {
                "music",
                {
                    {"type", "custom"},
                    {"url", url},
                    {"audio", audio_url},
                    {"title", title},
                    {"content", content},
                    {"image", image_url},
                },
            };
        }
    };

    struct Message : std::list<MessageSegment> {
        Message() = default;

        // 将字符串形式的消息转换为 Message 对象
        Message(const std::string &msg_str); // 实现在 cpp 文件
        Message(const char *msg_str) : Message(std::string(msg_str)) {
        }

        // 将消息段转换为 Message 对象
        Message(const MessageSegment &seg) {
            this->push_back(seg);
        }

        // 将 Message 对象转换为字符串形式的消息
        operator std::string() const {
            return std::transform_reduce(this->begin(), this->end(), std::string(), std::plus<>(), [](const auto &seg) {
                return std::string(seg);
            });
        }

        // 向指定主体发送消息
        int64_t send(const Target &target) const {
            return send_message(target, *this);
        }

        // 提取消息中的纯文本部分
        std::string extract_plain_text() const {
            std::string result;
            for (const auto &seg : *this) {
                if (seg.type == "text") {
                    result += seg.data.at("text") + " ";
                }
            }
            if (!result.empty()) {
                result.erase(result.end() - 1); // remove the trailing space
            }
            return result;
        }

        // 获取消息段链表的引用
        std::list<MessageSegment> &segments() {
            return *this;
        }

        // 获取消息段链表的常量引用
        const std::list<MessageSegment> &segments() const {
            return *this;
        }

        // 合并相邻的 text 消息段
        void reduce() {
            if (this->empty()) {
                return;
            }

            auto last_seg_it = this->begin();
            for (auto it = this->begin(); ++it != this->end();) {
                if (it->type == "text" && last_seg_it->type == "text" && it->data.find("text") != it->data.end()
                    && last_seg_it->data.find("text") != last_seg_it->data.end()) {
                    // found adjacent "text" segments
                    last_seg_it->data["text"] += it->data["text"];
                    // remove the current element and continue
                    this->erase(it);
                    it = last_seg_it;
                } else {
                    last_seg_it = it;
                }
            }

            if (this->size() == 1 && this->front().type == "text" && this->extract_plain_text().empty()) {
                this->clear(); // the only item is an empty text segment, we should remove it
            }
        }

        Message &operator+=(const Message &other) {
            this->insert(this->end(), other.begin(), other.end());
            this->reduce();
            return *this;
        }

        template <typename T>
        Message &operator+=(const T &other) {
            return this->operator+=(Message(other));
        }

        Message operator+(const Message &other) const {
            auto result = *this;
            result += other; // use operator+=
            return result;
        }

        template <typename T>
        Message operator+(const T &other) const {
            return this->operator+(Message(other));
        }
    };

    template <typename T>
    inline Message operator+(const T &lhs, const Message &rhs) {
        return Message(lhs) + rhs;
    }

    template <typename T>
    inline Message operator+(const MessageSegment &lhs, const T &rhs) {
        return Message(lhs) + rhs;
    }

    inline bool operator==(const MessageSegment &lhs, const MessageSegment &rhs) {
        return std::string(lhs) == std::string(rhs);
    }

    inline bool operator==(const Message &lhs, const Message &rhs) {
        return std::string(lhs) == std::string(rhs);
    }
} // namespace cq::message

namespace std {
    inline string to_string(const cq::message::Message &msg) {
        return string(msg);
    }
} // namespace std
