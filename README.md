# CoolQ C++ SDK

[![License](https://img.shields.io/github/license/cqmoe/cqcppsdk.svg)](LICENSE)
[![Tag](https://img.shields.io/github/v/tag/cqmoe/cqcppsdk)](https://github.com/cqmoe/cqcppsdk/releases)
[![QQ 群](https://img.shields.io/badge/qq%E7%BE%A4-590842375-orange.svg)](https://jq.qq.com/?_wv=1027&k=54T1cZB)
[![Telegram](https://img.shields.io/badge/telegram-chat-blue.svg)](https://t.me/cqhttp)
[![QQ 版本发布群](https://img.shields.io/badge/%E7%89%88%E6%9C%AC%E5%8F%91%E5%B8%83%E7%BE%A4-218529254-green.svg)](https://jq.qq.com/?_wv=1027&k=5Nl0zhE)
[![Telegram 版本发布频道](https://img.shields.io/badge/%E7%89%88%E6%9C%AC%E5%8F%91%E5%B8%83%E9%A2%91%E9%81%93-join-green.svg)](https://t.me/cqhttp_release)

CoolQ C++ SDK（或称 cqcppsdk、CQCPPSDK）是为了方便使用 C++ 开发 酷Q 应用而生的一个开发框架，封装了与 酷Q 提供的 DLL 接口相关的底层逻辑，对外提供更现代的 C++ 接口，从而提高应用开发效率，与此同时保持 C++ native 的性能优势。

## 极简示例

```cpp
#include <cqcppsdk/cqcppsdk.hpp>

using namespace cq;

CQ_INIT {
    on_private_message([](const auto &event) {
        try {
            send_message(event.target, event.message);
        } catch (ApiError &err) {
            logging::warning("私聊", "复读失败");
        }
    });
}
```

## 使用方法

请参考 [文档](https://cqcppsdk.cqp.moe/)。**请不要在阅读文档之前直接下载本仓库代码，除非你知道你在做什么。**

## 样例项目

样例 | 简介
--- | ---
[打包资源文件](https://github.com/cqmoe/cqcppsdk-example-packing-resources) | 将资源文件打包到 `app.dll`，并在运行时解包

## 应用案例

项目 | 简介
--- | ---
[richardchien/xiaofan-bot](https://github.com/richardchien/xiaofan-bot) | CQMOE 小秘书莫小凡
[dynilath/coolq-dicebot](https://github.com/dynilath/coolq-dicebot) | 骰子机器人
[MoeMod/CQuery](https://github.com/MoeMod/CQuery) | CS1.6/CSGO 服务器查询

## 问题反馈

如果使用过程中遇到任何问题、Bug，或有其它意见或建议，欢迎提 [issue](https://github.com/cqmoe/cqcppsdk/issues/new)。

也欢迎加入 QQ 交流群 590842375 来和大家讨论～
