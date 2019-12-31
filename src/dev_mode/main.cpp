#include "../core/common.h"

#include <iostream>

#include "../core/api.h"
#include "../core/event.h"
#include "../core/init.h"
#include "../utils/function.h"

using namespace std;
using namespace cq;

void prompt() { cout << ">>> "; }

int message_id = 0;

void process(string msg) {
    const int64_t user_id = 1;
    PrivateMessageEvent e;
    e.time = time(nullptr);
    e.sub_type = PrivateMessageEvent::SubType::FRIEND;
    e.target = Target::user(user_id);
    e.message_id = ++message_id;
    e.raw_message = msg; // TODO: 从控制台读取字符串可能不是 UTF-8
    e.message = e.raw_message;
    e.font = 0;
    e.user_id = user_id;
    utils::call_all(_private_message_callbacks, e);
}

int main() {
    cq::__init();
    cq::__init_api();

    string line;
    prompt();
    while (getline(cin, line)) {
        if (line.empty()) continue;
        process(line);
        prompt();
    }

    return 0;
}
