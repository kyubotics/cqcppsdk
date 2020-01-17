#include <csignal>
#include <iostream>

#include "../core/api.h"
#include "../core/common.h"
#include "../core/event.h"
#include "../core/event_callback.h"
#include "../core/init.h"
#include "../utils/function.h"
#include "./mock.h"

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#undef IGNORE // 避免名字冲突
#include <fcntl.h>
#include <io.h>
#endif

using namespace std;
using namespace cq;
using cq::utils::call_all;
using cq::utils::ws2s;

static void prompt() {
    cout << endl << ">>> ";
}

static void process(string msg) {
    static int64_t message_id = 0;

    auto e =
        PrivateMessageEvent(++message_id, std::move(msg), 0, FAKE_OTHER_USER_ID, PrivateMessageEvent::SubType::FRIEND);
    call_all(_private_message_callbacks, e);
    call_all(_message_callbacks, e);
}

static unsigned old_code_page;

static void exit_callback() {
    call_all(cq::_coolq_exit_callbacks);

#ifdef WIN32
    // 恢复控制台代码页
    SetConsoleCP(old_code_page);
    SetConsoleOutputCP(old_code_page);
#endif
}

static void sig_handler(int signum) {
    exit_callback();
}

int main() {
#ifdef WIN32
    // 保存当前控制台代码页
    old_code_page = GetConsoleCP();
    // 设置控制台代码页为 UTF-8
    SetConsoleCP(CP_UTF8);
    SetConsoleOutputCP(CP_UTF8);
    // 设置读取模式为宽字符
    _setmode(_fileno(stdin), _O_WTEXT);
#endif

    cq::__init();
    cq::__init_api();

    call_all(cq::_initialize_callbacks);
    call_all(cq::_coolq_start_callbacks);
    call_all(cq::_enable_callbacks);

    signal(SIGINT, sig_handler);

#ifdef WIN32
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

    exit_callback();
    return 0;
}
