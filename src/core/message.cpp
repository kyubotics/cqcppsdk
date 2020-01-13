#include "./message.h"

#include <sstream>

#include "../utils/string.h"

using namespace std;

namespace cq::message {
    using cq::utils::string_replace;
    using cq::utils::string_trim;

    // 对字符串做 CQ 码转义
    string escape(const std::string &str, const bool escape_comma) {
        string res = str;
        string_replace(res, "&", "&amp;");
        string_replace(res, "[", "&#91;");
        string_replace(res, "]", "&#93;");
        if (escape_comma) string_replace(res, ",", "&#44;");
        return res;
    }

    // 对字符串做 CQ 码去转义
    string unescape(const std::string &str) {
        string res = str;
        string_replace(res, "&#44;", ",");
        string_replace(res, "&#91;", "[");
        string_replace(res, "&#93;", "]");
        string_replace(res, "&amp;", "&");
        return res;
    }

    Message::Message(const std::string &msg_str) {
        // 定义字符流操作
        size_t idx = 0;
        const auto has_next = [&] { return idx < msg_str.length(); };
        const auto next = [&] { return msg_str[idx++]; };
        const auto move_rel = [&](const size_t rel_steps = 0) { idx += rel_steps; };
        const auto peek = [&] { return msg_str[idx]; };
        const auto peek_n = [&](const size_t count = 1) {
            return msg_str.substr(idx, min(count, msg_str.length() - idx));
        };

        // 判断当前位置是否 CQ 码开头
        const auto is_cq_code_begin = [&](const char ch) { return ch == '[' && peek_n(3) == "CQ:"; };

        // 定义状态
        static const auto S0 = 0;
        static const auto S1 = 1;
        auto state = S0;

        string temp_text; // 暂存以后可能作为 text 类型消息段保存的内容
        string cq_code; // 不包含 [CQ: 和 ] 的 CQ 码内容, 如 image,file=abc.jpg

        const auto save_temp_text = [&] {
            if (!temp_text.empty()) this->push_back(MessageSegment::text(unescape(temp_text)));
            temp_text.clear();
            cq_code.clear();
        };

        const auto save_cq_code = [&] {
            istringstream iss(cq_code);
            string type, param;
            map<string, string> data;
            getline(iss, type, ','); // 读取功能名
            while (iss) {
                getline(iss, param, ','); // 读取一个参数
                string_trim(param);
                if (param.empty()) continue;
                const auto eq_pos = param.find('=');
                data.emplace(string(param.begin(), param.begin() + eq_pos),
                             eq_pos != string::npos ? string(param.begin() + eq_pos + 1, param.end()) : "");
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
            default:
                break;
            }
        }
        save_temp_text(); // 保存剩余的临时文本
        this->reduce();
    }
} // namespace cq::message
