#pragma once

#include "common.hpp"

#include "api.hpp"

namespace cq::message {
    // 对字符串做 CQ 码转义
    inline std::string escape(const std::string &str, const bool escape_comma = true) {
        using cq::utils::string_replace;

        std::string res = str;
        string_replace(res, "&", "&amp;");
        string_replace(res, "[", "&#91;");
        string_replace(res, "]", "&#93;");
        if (escape_comma) string_replace(res, ",", "&#44;");
        return res;
    }

    // 对字符串做 CQ 码去转义
    inline std::string unescape(const std::string &str) {
        using cq::utils::string_replace;

        std::string res = str;
        string_replace(res, "&#44;", ",");
        string_replace(res, "&#91;", "[");
        string_replace(res, "&#93;", "]");
        string_replace(res, "&amp;", "&");
        return res;
    }

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
            return {"emoji", {{"id", to_string(id)}}};
        }

        // QQ 表情
        static MessageSegment face(const int id) {
            return {"face", {{"id", to_string(id)}}};
        }

        // 图片
        static MessageSegment image(const std::string &file) {
            return {"image", {{"file", file}}};
        }

        // 语音
        static MessageSegment record(const std::string &file, const bool magic = false) {
            return {"record", {{"file", file}, {"magic", to_string(magic)}}};
        }

        // @某人
        static MessageSegment at(const int64_t user_id) {
            return {"at", {{"qq", to_string(user_id)}}};
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
            return {"anonymous", {{"ignore", to_string(ignore_failure)}}};
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
                    {"id", to_string(id)},
                },
            };
        }

        // 位置
        static MessageSegment location(const double latitude, const double longitude, const std::string &title = "",
                                       const std::string &content = "") {
            return {
                "location",
                {
                    {"lat", to_string(latitude)},
                    {"lon", to_string(longitude)},
                    {"title", title},
                    {"content", content},
                },
            };
        }

        // 音乐
        static MessageSegment music(const std::string &type, const int64_t id) {
            return {"music", {{"type", type}, {"id", to_string(id)}}};
        }

        // 音乐
        static MessageSegment music(const std::string &type, const int64_t id, const int32_t style) {
            return {"music", {{"type", type}, {"id", to_string(id)}, {"style", to_string(style)}}};
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
        using std::list<MessageSegment>::list;

        // 将 C 字符串形式的消息转换为 Message 对象
        Message(const char *msg_str) : Message(std::string(msg_str)) {
        }

        // 将字符串形式的消息转换为 Message 对象
        Message(const std::string &msg_str) {
            using cq::utils::string_trim;

            // 定义字符流操作
            size_t idx = 0;
            const auto has_next = [&] { return idx < msg_str.length(); };
            const auto next = [&] { return msg_str[idx++]; };
            const auto move_rel = [&](const size_t rel_steps = 0) { idx += rel_steps; };
            const auto peek = [&] { return msg_str[idx]; };
            const auto peek_n = [&](const size_t count = 1) {
                return msg_str.substr(idx, std::min(count, msg_str.length() - idx));
            };

            // 判断当前位置是否 CQ 码开头
            const auto is_cq_code_begin = [&](const char ch) { return ch == '[' && peek_n(3) == "CQ:"; };

            // 定义状态
            enum { S0, S1 } state = S0;

            std::string temp_text; // 暂存以后可能作为 text 类型消息段保存的内容
            std::string cq_code; // 不包含 [CQ: 和 ] 的 CQ 码内容, 如 image,file=abc.jpg

            const auto save_temp_text = [&] {
                if (!temp_text.empty()) this->push_back(MessageSegment::text(unescape(temp_text)));
                temp_text.clear();
                cq_code.clear();
            };

            const auto save_cq_code = [&] {
                std::istringstream iss(cq_code);
                std::string type, param;
                std::map<std::string, std::string> data;
                getline(iss, type, ','); // 读取功能名
                while (iss) {
                    getline(iss, param, ','); // 读取一个参数
                    string_trim(param);
                    if (param.empty()) continue;
                    const auto eq_pos = param.find('=');
                    data.emplace(
                        std::string(param.begin(), param.begin() + eq_pos),
                        eq_pos != std::string::npos ? std::string(param.begin() + eq_pos + 1, param.end()) : "");
                    param.clear();
                }
                this->push_back(MessageSegment{std::move(type), std::move(data)});
                cq_code.clear();
                temp_text.clear();
            };

            /*
              状态图:
                 +---+            +---+
                 |   |            |   |
                 | other          | other
                 v   |            v   |
              +--+-+ |         +--+-+ |      +----+
              | S0 +-+--[CQ:-->+ S1 +-+--]-->+ SF |
              +--+-+           +--+-+        +----+
                 ^                |
                 |                |
                 +---[CQ:-back----+
            */
            while (has_next()) {
                const auto ch = next();
                switch (state) {
                case S0: // 处理纯文本或 CQ 码开头
                    if (is_cq_code_begin(ch)) {
                        // 潜在的 CQ 码开始
                        save_temp_text();
                        temp_text += "[CQ:";
                        move_rel(+3); // 跳过 CQ:
                        state = S1;
                    } else {
                        temp_text += ch;
                    }
                    break;
                case S1: // 处理 CQ 码内容
                    if (is_cq_code_begin(ch)) {
                        move_rel(-1); // 回退 [
                        state = S0; // 回到 S0
                    } else if (ch == ']') {
                        // CQ 码结束
                        save_cq_code();
                        state = S0;
                    } else {
                        cq_code += ch;
                        temp_text += ch;
                    }
                    break;
                }
            }
            save_temp_text(); // 保存剩余的临时文本
            this->reduce();
        }

        // 将消息段转换为 Message 对象
        Message(const MessageSegment &seg) {
            this->push_back(seg);
        }

        // 将 Message 对象转换为字符串形式的消息
        operator std::string() const {
            return std::accumulate(this->begin(), this->end(), std::string(), [](const auto &seg1, const auto &seg2) {
                return std::string(seg1) + std::string(seg2);
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
