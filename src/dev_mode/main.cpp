#include <ctime>
#include <iostream>
#include <string>

#include "cqcppsdk.h"

using namespace std;

void prompt() { cout << ">>> "; }

int message_id = 0;

void process(string msg) {
    const int64_t user_id = 1;
    cq::PrivateMessageEvent e;
    e.time = time(nullptr);
    e.sub_type = cq::PrivateMessageEvent::SubType::FRIEND;
    e.target = cq::Target::user(user_id);
    e.message_id = ++message_id;
    e.raw_message = msg;
    e.message = msg;
    e.user_id = user_id;
    for (auto handler : cq::_private_message_callbacks) {
        handler(e);
    }
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
