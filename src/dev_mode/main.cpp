#include "../core/common.h"

#include <signal.h>
#include <iostream>

#include "../core/api.h"
#include "../core/event.h"
#include "../core/init.h"
#include "../utils/function.h"
#include "./mock.h"

using namespace std;
using namespace cq;
using cq::utils::call_all;

static void prompt() { cout << endl << ">>> "; }

static void process(string msg) {
    static int message_id = 0;

    PrivateMessageEvent e;
    e.time = time(nullptr);
    e.sub_type = PrivateMessageEvent::SubType::FRIEND;
    e.target = Target::user(FAKE_SENDER_USER_ID);
    e.message_id = ++message_id;
    e.message = msg;
    e.font = 0;
    e.user_id = FAKE_SENDER_USER_ID;
    call_all(_private_message_callbacks, e);
}

static void sig_handler(int signum) { call_all(cq::_coolq_exit_callbacks); }

int main() {
    cq::__init();
    cq::__init_api();

    call_all(cq::_initialize_callbacks);
    call_all(cq::_coolq_start_callbacks);
    call_all(cq::_enable_callbacks);

    signal(SIGINT, sig_handler);

    string line;
    prompt();
    while (getline(cin, line)) {
        if (line.empty()) continue;
        process(line);
        prompt();
    }

    call_all(cq::_coolq_exit_callbacks);
    return 0;
}
