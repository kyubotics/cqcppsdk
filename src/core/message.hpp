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

    // 消息段 (即 CQ 码)
    class MessageSegment {
    public:
        enum class SegTypes {
#define MSG_SEG(val) val,
#include "./message_segment_types.inc"
#undef MSG_SEG
            unimpl
        };

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

    private:
        using value_type = std::string;
        using map_type = std::map<value_type, value_type>;
        using variant_type = ::std::variant<value_type, map_type>;

        // 消息段类型 (即 CQ 码的功能名)
        SegTypes _type = SegTypes::unimpl;

        // 当type为text和unimpl时，data为字符串
        // text为直接文本数据，unimpl为CQ码原文
        // 其他情况中，消息段数据 (即 CQ 码参数), 字符串全部使用未经 CQ 码转义的原始文本
        variant_type data;

        explicit MessageSegment(SegTypes t, map_type in_map) noexcept {
            this->_type = t;
            data = std::move(in_map);
        }

        explicit MessageSegment(SegTypes t, value_type in_string) noexcept {
            this->_type = t;
            data = std::move(in_string);
        }

        explicit MessageSegment(SegTypes t) noexcept {
            this->_type = t;
            data = map_type();
        }

        explicit MessageSegment(value_type in_string) noexcept {
            this->_type = SegTypes::unimpl;
            data = std::move(in_string);
        }

        inline void forward_variant(variant_type &&val) {
            switch (this->_type) {
            case SegTypes::text:
            case SegTypes::unimpl:
                this->data = ::std::get<value_type>(::std::move(val));
                break;
            default:
                this->data = ::std::get<map_type>(::std::move(val));
            }
        }

        inline void forward_variant(const variant_type &val) {
            this->forward_variant(variant_type(val));
        }

    public:
        MessageSegment(MessageSegment &&val) noexcept {
            *this = ::std::move(val);
        }
        MessageSegment(const MessageSegment &val) noexcept {
            *this = val;
        }
        MessageSegment &operator=(MessageSegment &&val) noexcept {
            this->_type = val._type;
            this->forward_variant(::std::forward<variant_type>(val.data));
            return *this;
        }
        MessageSegment &operator=(const MessageSegment &val) noexcept {
            this->_type = val._type;
            this->forward_variant(val.data);
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

        // 从cq码创建segment
        static MessageSegment fromCQCode(std::string cq_code) {
            auto find_char = [&](auto from, auto val) -> auto {
                return ::std::find(from, cq_code.end(), val);
            };

            auto rfind_char = [&](auto val) -> auto {
                return ::std::find(cq_code.rbegin(), cq_code.rend(), val);
            };

            auto is_end = [&](auto iter) -> bool { return iter == cq_code.end(); };

            {
                auto where_lbra = find_char(cq_code.begin(), '[');
                cq_code.erase(cq_code.begin(), where_lbra);

                auto rwhere_rbra = ::std::find(cq_code.rbegin(), cq_code.rend(), ']');
                if (rwhere_rbra == cq_code.rend()) throw ::std::invalid_argument("Invalid CQCode");

                auto where_rbra = rwhere_rbra.base();
                if (!is_end(where_rbra)) cq_code.erase(where_rbra, cq_code.end());
            }

            auto the_colon_after_CQ = find_char(cq_code.begin(), ':');
            if (is_end(the_colon_after_CQ)) throw ::std::invalid_argument("Invalid CQCode");

            auto the_first_comma = find_char(the_colon_after_CQ, ',');

            ::std::string type = is_end(the_first_comma)
                                     ? ::std::string(::std::next(the_colon_after_CQ), cq_code.end() - 2)
                                     : ::std::string(::std::next(the_colon_after_CQ), the_first_comma);
            auto type_iter = SegTypeName2SegTypes.find(type);
            if (type_iter == SegTypeName2SegTypes.end()) {
                return MessageSegment(::std::move(cq_code));
            } else {
                auto pos = the_first_comma;
                map_type temp_data;
                while (pos != cq_code.end()) {
                    auto equal_pos = find_char(pos, '=');
                    auto comma_pos = find_char(equal_pos, ',');
                    if (is_end(comma_pos)) {
                        temp_data.insert({std::string(::std::next(pos), equal_pos),
                                          std::string(::std::next(equal_pos), ::std::prev(cq_code.end()))});
                    } else {
                        temp_data.insert({std::string(::std::next(pos), equal_pos),
                                          std::string(::std::next(equal_pos), ::std::next(comma_pos))});
                    }
                    pos = comma_pos;
                }
                return MessageSegment(type_iter->second, ::std::move(temp_data));
            }
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

    struct Message : std::list<MessageSegment> {
        using std::list<MessageSegment>::list;

        // 将 C 字符串形式的消息转换为 Message 对象
        Message(const char *msg_str) : Message(std::string(msg_str)) {
        }

        // 将字符串形式的消息转换为 Message 对象
        Message(const std::string &msg_str) {
            using cq::utils::string_trim;

            const ::std::string CQ_head = "[CQ:";

            // 不属于CQ码的"["和"]"在CQ信息中总是会escape为"&#91"和"&#93"，算是好处理的地方
            auto search_cq_head = [&](auto from) -> auto {
                return ::std::search(from, msg_str.end(), CQ_head.begin(), CQ_head.end());
            };
            auto find_cq_tail = [&](auto from) -> auto {
                return ::std::find(from, msg_str.end(), ']');
            };
            auto is_end = [&](auto iter) -> bool { return iter == msg_str.end(); };

            auto work_pos = msg_str.begin();
            while (!is_end(work_pos)) {
                auto cq_head_pos = search_cq_head(work_pos);
                if (is_end(cq_head_pos)) {
                    this->push_back(MessageSegment::text(::std::string(work_pos, cq_head_pos)));
                    break;
                } else {
                    if (::std::distance(work_pos, cq_head_pos) > 0)
                        this->push_back(MessageSegment::text(::std::string(work_pos, cq_head_pos)));
                    auto cq_tail_pos = find_cq_tail(cq_head_pos);
                    this->push_back(MessageSegment::fromCQCode(::std::string(cq_head_pos, cq_tail_pos + 1)));
                    work_pos = ::std::next(cq_tail_pos);
                }
            }
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

        Message &operator+=(const Message &other) {
            auto start = other.begin();
            if (!this->empty() && !other.empty() && this->back().type() == other.front().type()
                && this->back().type() == MessageSegment::SegTypes::text) {
                this->back() = MessageSegment::text(this->back().plain_text() + other.front().plain_text());
                ::std::advance(start, 1);
            }
            this->insert(this->end(), start, other.end());
            return *this;
        }

        template <typename Tx,
                  typename ::std::enable_if_t<::std::is_same_v<::std::decay_t<Tx>, MessageSegment>, int> = 0>
        Message &operator+=(Tx &&segment) {
            return this->push_back(::std::forward(segment));
        }

        Message operator+(const Message &other) const {
            auto result = *this;
            result += other; // use operator+=
            return result;
        }
    };

    template <
        typename T, typename Tx,
        typename ::std::enable_if_t<::std::is_convertible_v<T, Message> && !::std::is_same_v<T, Message>, int> = 0,
        typename ::std::enable_if_t<::std::is_same_v<::std::decay_t<Tx>, MessageSegment>, int> = 0>
    inline Message operator+(const T &lhs, Tx &&rhs) {
        return Message(lhs) + ::std::forward<MessageSegment>(rhs);
    }

    template <typename T, typename Tx,
              typename ::std::enable_if_t<::std::is_same_v<::std::decay_t<Tx>, MessageSegment>, int> = 0>
    inline Message operator+(Tx &&lhs, const T &rhs) {
        return Message(::std::forward<MessageSegment>(lhs)) + rhs;
    }

    inline bool operator==(const MessageSegment &lhs, const MessageSegment &rhs) {
        return lhs.type() == rhs.type() && ::std::string(lhs) == ::std::string(rhs);
    }

    inline bool operator!=(const MessageSegment &lhs, const MessageSegment &rhs) {
        return !(lhs == rhs);
    }

    inline bool operator==(const Message &lhs, const Message &rhs) {
        if (lhs.size() != rhs.size()) return false;
        auto lhs_iter = lhs.begin();
        auto rhs_iter = rhs.begin();
        while (lhs_iter != lhs.end() && rhs_iter != rhs.end()) {
            if (*lhs_iter != *rhs_iter) return false;
            ::std::advance(lhs_iter, 1);
            ::std::advance(rhs_iter, 1);
        }
        return true;
    }
    inline bool operator!=(const Message &lhs, const Message &rhs) {
        return !(lhs == rhs);
    }
} // namespace cq::message
