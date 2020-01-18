# CoolQ C++ SDK

[![License](https://img.shields.io/github/license/cqmoe/cqcppsdk.svg)](LICENSE)
[![QQ 群](https://img.shields.io/badge/qq%E7%BE%A4-590842375-orange.svg)](https://jq.qq.com/?_wv=1027&k=54T1cZB)
[![Telegram](https://img.shields.io/badge/telegram-chat-blue.svg)](https://t.me/cqhttp)
[![QQ 版本发布群](https://img.shields.io/badge/%E7%89%88%E6%9C%AC%E5%8F%91%E5%B8%83%E7%BE%A4-218529254-green.svg)](https://jq.qq.com/?_wv=1027&k=5Nl0zhE)
[![Telegram 版本发布频道](https://img.shields.io/badge/%E7%89%88%E6%9C%AC%E5%8F%91%E5%B8%83%E9%A2%91%E9%81%93-join-green.svg)](https://t.me/cqhttp_release)

CoolQ C++ SDK（或称 cqcppsdk、CQCPPSDK）是为了方便使用 C++ 开发 酷Q 应用而生的一个开发框架，封装了与 酷Q 提供的 DLL 接口相关的底层逻辑，对外提供更现代的 C++ 接口，从而提高应用开发效率，与此同时保持 C++ native 的性能优势。

## 示例

```cpp
#include <iostream>
#include <set>
#include <sstream>

#include <cqcppsdk/cqcppsdk.h>

using namespace cq;
using namespace std;

CQ_INIT {
    on_enable([] { logging::info("启用", "插件已启用"); });

    on_private_message([](const PrivateMessageEvent &e) {
        try {
            auto msgid = send_private_message(e.user_id, e.message); // 直接复读消息
            logging::info_success("私聊", "私聊消息复读完成, 消息 Id: " + to_string(msgid));
        } catch (ApiError &e) {
            logging::warning("私聊", "私聊消息复读失败, 错误码: " + e.code);
        }
    });

    on_group_upload([](const auto &e) { // 可以使用 auto 自动推断类型
        stringstream ss;
        ss << "您上传了一个文件, 文件名: " << e.file.name << ", 大小(字节): " << e.file.size;
        try {
            send_message(e.target, ss.str());
        } catch (ApiError &) {
        }
    });
}

CQ_MENU(menu_demo_1) { logging::info("菜单", "点击菜单1"); }
```

## 使用方法

请参考 [文档](https://cqcppsdk.cqp.moe/)。**请不要在阅读文档之前直接下载本仓库代码，除非你知道你在做什么。**

## 问题反馈

如果使用过程中遇到任何问题、Bug，或有其它意见或建议，欢迎提 [issue](https://github.com/cqmoe/cqcppsdk/issues/new)。

也欢迎加入 QQ 交流群 590842375 来和大家讨论～
