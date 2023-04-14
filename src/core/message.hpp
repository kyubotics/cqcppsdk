#pragma once

#include "common.hpp"

#include "api.hpp"

#include <variant>

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

    class Message;

    // 消息段 (即 CQ 码)
    class MessageSegment {
    public:
        enum class SegTypes {
#define MSG_SEG(val) val,
#include "./message_segment_types.inc"
#undef MSG_SEG
            unimpl
        };

    private:
        inline static constexpr char *const SegTypesName[] = {
#define MSG_SEG(val) #val,
#include "./message_segment_types.inc"
#undef MSG_SEG
            ""};

        inline static const ::std::unordered_map<::std::string, MessageSegment::SegTypes> SegTypeName2SegTypes = {
#define MSG_SEG(val) {#val, MessageSegment::SegTypes::val},
#include "./message_segment_types.inc"
#undef MSG_SEG
            {"", MessageSegment::SegTypes::unimpl}};

        using value_type = std::string;
        using map_type = std::map<value_type, value_type>;
        using variant_type = ::std::variant<value_type, map_type>;

        // 消息段类型 (即 CQ 码的功能名)
        SegTypes _type;

        // 当type为text和unimpl时，data为字符串
        // text为直接文本数据，unimpl为CQ码原文
        // 其他情况中，消息段数据 (即 CQ 码参数), 字符串全部使用未经 CQ 码转义的原始文本
        variant_type data;

        // 构造支持的键值对数据段
        explicit MessageSegment(SegTypes t, map_type in_map) noexcept {
            this->_type = t;
            data = std::move(in_map);
        }

        // 构造支持的字符串数据段，仅有text
        explicit MessageSegment(SegTypes t, value_type in_string) noexcept {
            this->_type = t;
            data = std::move(in_string);
        }

        // 构造仅有type的段
        explicit MessageSegment(SegTypes t) noexcept {
            this->_type = t;
            data = map_type();
        }

        // 构造不支持的段
        explicit MessageSegment(value_type in_string) noexcept {
            this->_type = SegTypes::unimpl;
            data = std::move(in_string);
        }

        friend Message;

        inline static bool testCQCode(const ::std::string &src) noexcept {
            constexpr char CQ_head[] = "[CQ:";
            constexpr char CQ_end = ']';
            if (src.size() <= sizeof(CQ_head) + sizeof(CQ_end)) return false;
            for (size_t i = 0; !CQ_head[i]; i++)
                if (src[i] != CQ_head[i]) return false;
            if (src.back() != CQ_end) return false;
            auto cursor = src.begin();
            auto expect = [&](char delim) -> bool {
                cursor = ::std::find(cursor, src.end(), delim);
                return cursor != src.end();
            };
            while (!expect(',')) {
                if (!expect('=')) return false;
            }
            return true;
        }

        // 从cq码创建segment，会报错
        // MessageSegment总是对自身内部数据保证所有权，所以按值传参
        static MessageSegment fromCQCodeNoCheck(::std::string cq_code) noexcept(false) {
            auto find_char = [&](auto from, auto val) -> auto {
                return ::std::find(from, cq_code.end(), val);
            };

            auto rfind_char = [&](auto from, auto val) -> auto {
                return ::std::find(from, cq_code.rend(), val);
            };

            auto is_end = [&](auto iter) -> bool { return iter == cq_code.end(); };

#define DECQCODE_ASSERT(test)                                         \
    {                                                                 \
        if (!(test)) throw ::std::invalid_argument("Invalid CQCode"); \
    }
            DECQCODE_ASSERT(cq_code.front() == '[')
            DECQCODE_ASSERT(cq_code.back() == ']');

            auto the_colon_after_CQ = find_char(cq_code.begin(), ':');
            DECQCODE_ASSERT(the_colon_after_CQ != cq_code.end());

            auto rbracket_pos = ::std::prev(cq_code.end());

            auto the_first_comma = find_char(the_colon_after_CQ, ',');
            ::std::string type;
            if (is_end(the_first_comma)) {
                ::std::string(::std::next(the_colon_after_CQ), rbracket_pos);
            } else {
                type = ::std::string(::std::next(the_colon_after_CQ), the_first_comma);
            }

            auto type_iter = SegTypeName2SegTypes.find(type);
            if (type_iter == SegTypeName2SegTypes.end()) {
                return MessageSegment(::std::move(cq_code));
            } else {
                auto pos = the_first_comma;
                map_type temp_data;
                while (pos != cq_code.end()) {
                    auto equal_pos = find_char(pos, '=');
                    _ASSERT(equal_pos != cq_code.end());
                    auto comma_pos = find_char(equal_pos, ',');
                    if (is_end(comma_pos)) {
                        temp_data.insert({std::string(::std::next(pos), equal_pos),
                                          std::string(::std::next(equal_pos), ::std::prev(cq_code.end()))});
                    } else {
                        temp_data.insert({std::string(::std::next(pos), equal_pos),
                                          std::string(::std::next(equal_pos), ::std::next(rbracket_pos))});
                        break;
                    }
                    pos = comma_pos;
                }
                return MessageSegment(type_iter->second, ::std::move(temp_data));
            }
#undef DECQCODE_ASSERT
        }

    public:
        MessageSegment() noexcept {
            this->_type = SegTypes::unimpl;
        }
        MessageSegment(MessageSegment &&val) noexcept {
            this->_type = val._type;
            this->data = ::std::move(val.data);
        }
        MessageSegment(const MessageSegment &val) noexcept {
            this->_type = val._type;
            this->data = val.data;
        }
        MessageSegment &operator=(MessageSegment &&val) noexcept {
            this->_type = val._type;
            this->data = ::std::move(val.data);
            return *this;
        }
        MessageSegment &operator=(const MessageSegment &val) noexcept {
            this->_type = val._type;
            this->data = val.data;
            return *this;
        }

        // 获得segment的类型，unimpl类型表示该CQ码在sdk中未提供构造方法
        // 其他类型均有提供对应的构造方法，例如MessageSegment::SegTypes::face对应有MessageSegment::face方法
        inline SegTypes type() const {
            return this->_type;
        }

        // 获得segment的类型对应的字符串
        std::string segTypeName() const {
            return SegTypesName[static_cast<size_t>(this->_type)];
        }

        // 提供==语义
        inline bool operator==(const MessageSegment &other) const noexcept {
            return this->type() == other.type() && this->data == other.data;
        }

        // 提供!=语义
        inline bool operator!=(const MessageSegment &other) const noexcept {
            return !this->operator==(other);
        }

        // 从cq码创建segment
        static MessageSegment fromCQCode(std::string cq_code) {
            if (!testCQCode(cq_code)) return MessageSegment();
            return MessageSegment::fromCQCodeNoCheck(cq_code);
        }

        // 转换为字符串形式
        operator std::string() const {
            std::string s;
            switch (this->_type) {
            case SegTypes::text:
            case SegTypes::unimpl: {
                return ::std::get<value_type>(this->data);
            }
            default: {
                auto &data_map = ::std::get<map_type>(this->data);
                s += "[CQ:" + this->segTypeName();
                for (const auto &item : data_map) {
                    s += "," + item.first + "=" + escape(item.second, true);
                }
                s += "]";
                return s;
            }
            }
        }

        // 获得MessageSegment中的键值对，对于text和unimpl的Segment将抛出错误
        // 只提供返回常量引用，需要不同的内容应当另外构造
        inline const map_type &value_map() const {
            return ::std::get<map_type>(this->data);
        }

        // 获得MessageSegment中的文本，对于非text且非unimpl的Segment将抛出错误
        // 只提供返回常量引用，需要不同的内容应当另外构造
        inline const value_type &plain_text() const {
            return ::std::get<value_type>(this->data);
        }

        // 纯文本
        static MessageSegment text(const std::string &text) {
            return MessageSegment(SegTypes::text, text);
        }

        // Emoji 表情
        static MessageSegment emoji(const uint32_t id) {
            return MessageSegment(SegTypes::emoji, {{"id", to_string(id)}});
        }

        // QQ 表情
        static MessageSegment face(const int id) {
            return MessageSegment(SegTypes::face, {{"id", to_string(id)}});
        }

        // 图片
        static MessageSegment image(const std::string &file) {
            return MessageSegment(SegTypes::image, {{"file", file}});
        }

        // 语音
        static MessageSegment record(const std::string &file, const bool magic = false) {
            return MessageSegment(SegTypes::record, {{"file", file}, {"magic", to_string(magic)}});
        }

        // @某人
        static MessageSegment at(const int64_t user_id) {
            return MessageSegment(SegTypes::at, {{"qq", to_string(user_id)}});
        }

        // 猜拳魔法表情
        static MessageSegment rps() {
            return MessageSegment(SegTypes::rps);
        }

        // 掷骰子魔法表情
        static MessageSegment dice() {
            return MessageSegment(SegTypes::dice);
        }

        // 戳一戳
        static MessageSegment shake() {
            return MessageSegment(SegTypes::shake);
        }

        // 匿名发消息
        static MessageSegment anonymous(const bool ignore_failure = false) {
            return MessageSegment(SegTypes::anonymous, {{"ignore", to_string(ignore_failure)}});
        }

        // 链接分享
        static MessageSegment share(const std::string &url, const std::string &title, const std::string &content = "",
                                    const std::string &image_url = "") {
            return MessageSegment(SegTypes::share,
                                  {{"url", url}, {"title", title}, {"content", content}, {"image", image_url}});
        }

        enum class ContactType { USER, GROUP };

        // 推荐好友, 推荐群
        static MessageSegment contact(const ContactType &type, const int64_t id) {
            return MessageSegment(SegTypes::contact,
                                  {
                                      {"type", type == ContactType::USER ? "qq" : "group"},
                                      {"id", to_string(id)},
                                  });
        }

        // 位置
        static MessageSegment location(const double latitude, const double longitude, const std::string &title = "",
                                       const std::string &content = "") {
            return MessageSegment(SegTypes::location,
                                  {
                                      {"lat", to_string(latitude)},
                                      {"lon", to_string(longitude)},
                                      {"title", title},
                                      {"content", content},
                                  });
        }

        // 音乐
        static MessageSegment music(const std::string &type, const int64_t id) {
            return MessageSegment(SegTypes::music, {{"type", type}, {"id", to_string(id)}});
        }

        // 音乐
        static MessageSegment music(const std::string &type, const int64_t id, const int32_t style) {
            return MessageSegment(SegTypes::music,
                                  {{"type", type}, {"id", to_string(id)}, {"style", to_string(style)}});
        }

        // 音乐自定义分享
        static MessageSegment music(const std::string &url, const std::string &audio_url, const std::string &title,
                                    const std::string &content = "", const std::string &image_url = "") {
            return MessageSegment(SegTypes::music,
                                  {
                                      {"type", "custom"},
                                      {"url", url},
                                      {"audio", audio_url},
                                      {"title", title},
                                      {"content", content},
                                      {"image", image_url},
                                  });
        }
    }; // namespace cq::message

    class Message : public std::list<MessageSegment> {
    private:
        using container_type = std::list<MessageSegment>;

        inline static MessageSegment fromCQCodeNoCheck(const ::std::string &cq_code) {
            return MessageSegment::fromCQCodeNoCheck(cq_code);
        }

    public:
        Message() noexcept {};

        // 将 C 字符串形式的消息转换为 Message 对象
        Message(const char *msg_str) : Message(std::string(msg_str)) {
        }

        // 将字符串形式的消息转换为 Message 对象
        // 如果字符串中CQ码不符合规范，会抛出invalid_argument，此时构造的Message中无元素
        Message(const std::string &msg_str) {
            using cq::utils::string_trim;

            const ::std::string CQ_head = "[CQ:";

            auto find_char = [&](auto from, char delim) -> auto {
                return ::std::find(from, msg_str.end(), delim);
            };
            auto is_end = [&](auto iter) -> bool { return iter == msg_str.end(); };

            container_type cont;
            auto work_pos = msg_str.begin();
            while (!is_end(work_pos)) {
                // 不属于CQ码的"["和"]"在CQ信息中总是会escape为"&#91"和"&#93"，算是好处理的地方
                auto cq_head_pos = find_char(work_pos, '[');
                if (is_end(cq_head_pos)) {
                    cont.push_back(MessageSegment::text(::std::string(work_pos, cq_head_pos)));
                    break;
                } else {
                    if (::std::distance(work_pos, cq_head_pos) > 0)
                        cont.push_back(MessageSegment::text(::std::string(work_pos, cq_head_pos)));

                    // 由于可以从用户指定的字符串构建，所以有可能先遇到'['
                    auto cq_tail_pos = ::std::find_if(
                        ::std::next(cq_head_pos), msg_str.end(), [&](char w) -> bool { return w == '[' || w == ']'; });

                    if (!is_end(cq_tail_pos) && *cq_tail_pos == ']') cq_tail_pos = ::std::next(cq_tail_pos);

                    // 如果先遇到'['，会发生异常，Message会保持空容器
                    cont.push_back(MessageSegment::fromCQCodeNoCheck(::std::string(cq_head_pos, cq_tail_pos)));
                    work_pos = cq_tail_pos;
                }
            }
            // fromCQCodeNoCheck可能抛出，使用swap来保证强异常安全
            this->swap(cont);
        }

        // 将消息段转换为 Message 对象
        Message(MessageSegment seg) {
            this->push_back(::std::move(seg));
        }

        // 将 Message 对象转换为字符串形式的消息
        operator std::string() const noexcept {
            ::std::ostringstream oss;
            for (auto &seg : *this) {
                oss << ::std::string(seg);
            }
            return oss.str();
        }

        // 向指定主体发送消息
        int64_t send(const Target &target) const {
            return send_message(target, *this);
        }

        // 提取消息中的纯文本部分
        std::string extract_plain_text() const {
            std::string result;
            for (const auto &seg : *this) {
                if (seg.type() == MessageSegment::SegTypes::text) {
                    result += seg.plain_text();
                }
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
            if (this->empty()) return;

            auto iter_last = this->begin();

            while (iter_last != this->end()) {
                iter_last = ::std::find_if(iter_last, this->end(), [](auto &val) -> bool {
                    return val.type() == MessageSegment::SegTypes::text;
                });
                if (iter_last == this->end()) break;

                std::string sum = iter_last->plain_text();
                auto iter_this = ::std::next(iter_last);

                while (iter_this != this->end() && iter_this->type() == MessageSegment::SegTypes::text) {
                    sum += iter_this->plain_text();
                    this->erase(iter_this);
                    iter_this = ::std::next(iter_last);
                }
                if (iter_last->plain_text().size() != sum.size()) *iter_last = MessageSegment::text(sum);
                iter_last = iter_this;
            }
        }

        // 连接另一个Message
        inline Message &operator+=(Message other) {
            auto start = other.begin();
            if (!this->empty() && !other.empty() && this->back().type() == other.front().type()
                && this->back().type() == MessageSegment::SegTypes::text) {
                this->back() = MessageSegment::text(this->back().plain_text() + other.front().plain_text());
                ::std::advance(start, 1);
            }
            this->splice(this->end(), other, start, other.end());
            return *this;
        }

        // 连接另一个MessageSegment
        inline Message &operator+=(MessageSegment segment) {
            this->push_back(::std::move(segment));
            return *this;
        }

        // 提供==语义
        inline bool operator==(const Message &rhs) {
            if (this->size() != rhs.size()) return false;
            auto lhs_iter = this->begin();
            auto rhs_iter = rhs.begin();
            while (lhs_iter != this->end() && rhs_iter != rhs.end()) {
                if (*lhs_iter != *rhs_iter) return false;
                ::std::advance(lhs_iter, 1);
                ::std::advance(rhs_iter, 1);
            }
            return true;
        }

        // 提供!=语义
        inline bool operator!=(const Message &rhs) {
            return !this->operator==(rhs);
        }
    };

    // 提供任何能转换到Message的对象和MessageSegment之间的连接运算
    inline Message operator+(Message lhs, MessageSegment rhs) {
        return lhs += ::std::move(rhs);
    }

    // 提供任何能转换到Message的对象和Message之间的连接运算
    inline Message operator+(Message lhs, Message rhs) {
        return lhs += ::std::move(rhs);
    }
} // namespace cq::message
