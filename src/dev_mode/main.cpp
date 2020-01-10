#include <signal.h>
#include <iostream>

#ifdef WIN32
#include <fcntl.h>
#include <io.h>
#endif

#include "../core/api.h"
#include "../core/common.h"
#include "../core/event.h"
#include "../core/init.h"
#include "../utils/function.h"
#include "./mock.h"

using namespace std;
using namespace cq;
using cq::utils::call_all;
using cq::utils::ws2s;

static void prompt() {
    cout << endl << ">>> ";
}

static void process(const string &msg) {
    static int64_t message_id = 0;

    auto e = PrivateMessageEvent(++message_id, msg, 0, FAKE_OTHER_USER_ID, PrivateMessageEvent::SubType::FRIEND);
    call_all(_private_message_callbacks, e);
}

static void sig_handler(int signum) {
    call_all(cq::_coolq_exit_callbacks);
}

int main() {
    cq::__init();
    cq::__init_api();

    call_all(cq::_initialize_callbacks);
    call_all(cq::_coolq_start_callbacks);
    call_all(cq::_enable_callbacks);

    signal(SIGINT, sig_handler);

#ifdef WIN32
    _setmode(_fileno(stdin), _O_WTEXT);
    wstring line;
    prompt();
    while (getline(wcin, line)) {
        if (line.empty()) continue;
        process(ws2s(line));
        prompt();
    }
#else
    string line;
    prompt();
    while (getline(cin, line)) {
        if (line.empty()) continue;
        process(line);
        prompt();
    }
#endif

    call_all(cq::_coolq_exit_callbacks);
    return 0;
}
