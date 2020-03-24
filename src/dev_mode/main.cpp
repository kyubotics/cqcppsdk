#include <csignal>
#include <iostream>

#include "mock.hpp"

#include "../core/api.hpp"
#include "../core/event.hpp"
#include "../core/event_callback.hpp"
#include "../core/init.hpp"
#include "../utils/function.hpp"

#if defined(WIN32) && !(defined(__GNUC__) || defined(__MINGW32__))
#define WIN32_NO_MINGW
#endif

#ifdef WIN32_NO_MINGW
#define WIN32_LEAN_AND_MEAN
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#undef IGNORE // 避免名字冲突
#include <fcntl.h>
#include <io.h>
#endif

using namespace cq;
using std::cout;
using std::endl;
using std::string;
using std::cin;
using std::wcin;
using std::wstring;
using std::getline;
using cq::utils::call_all;
using cq::utils::ws2s;

static void prompt() {
    cout << endl << ">>> ";
}

static void process(string msg) {
    static int64_t message_id = 0;

    auto e =
        PrivateMessageEvent(FAKE_OTHER_USER_ID, ++message_id, std::move(msg), 0, PrivateMessageEvent::SubType::FRIEND);
    call_all(cq::_private_message_callbacks(), e);
    call_all(cq::_message_callbacks(), e);
}

static unsigned old_code_page;

static void exit_callback() {
    call_all(cq::_coolq_exit_callbacks());

#ifdef WIN32_NO_MINGW
    // 恢复控制台代码页
    SetConsoleCP(old_code_page);
    SetConsoleOutputCP(old_code_page);
#endif
}

static void sig_handler(int) {
    exit_callback();
}

int main() {
#ifdef WIN32_NO_MINGW
    // 保存当前控制台代码页
    old_code_page = GetConsoleCP();
    // 设置控制台代码页为 UTF-8
    SetConsoleCP(CP_UTF8);
    SetConsoleOutputCP(CP_UTF8);
    // 设置读取模式为宽字符
    _setmode(_fileno(stdin), _O_WTEXT);
#endif

    cq::_init();
    cq::_init_api();

    call_all(cq::_initialize_callbacks());
    call_all(cq::_coolq_start_callbacks());
    call_all(cq::_enable_callbacks());

    signal(SIGINT, sig_handler);

#ifdef WIN32_NO_MINGW
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
