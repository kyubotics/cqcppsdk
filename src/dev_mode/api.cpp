/**
 * 本文件提供 api.h 的 mock 实现, 仅输出日志并返回模拟数据, 可在任何平台编译.
 */

#include "../core/api.h"

#include <iostream>

using namespace std;

namespace cq {
    void __init_api() {}

    static int64_t message_id = 0;

    int64_t send_private_message(const int64_t user_id, const std::string &message) {
        cout << "send_private_message" << endl;
        cout << "  user_id: " << user_id << endl;
        cout << "  message: " << message << endl;
        return ++message_id;
    }

    int64_t send_group_message(const int64_t group_id, const std::string &message) {
        cout << "send_group_message" << endl;
        cout << "  group_id: " << group_id << endl;
        cout << "  message: " << message << endl;
        return ++message_id;
    }

    int64_t send_discuss_message(const int64_t discuss_id, const std::string &message) {
        cout << "send_discuss_message" << endl;
        cout << "  discuss_id: " << discuss_id << endl;
        cout << "  message: " << message << endl;
        return ++message_id;
    }
} // namespace cq
