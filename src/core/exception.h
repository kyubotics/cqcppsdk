#pragma once

#include <exception>
#include <string>

namespace cq {
    // cq 命名空间下所有异常类型的基类
    struct Exception : std::exception {
        using std::exception::exception;

        explicit Exception(const string &message) : std::exception(message.c_str()) {
        }
    };

    // 用户用法错误导致的异常
    struct LogicError : Exception {
        using Exception::Exception;
    };

    // 用户无法控制的运行时异常
    struct RuntimeError : Exception {
        using Exception::Exception;
    };

    // 从序列化数据中解析内容出错导致的异常
    struct ParseError : RuntimeError {
        using RuntimeError::RuntimeError;
    };
} // namespace cq
