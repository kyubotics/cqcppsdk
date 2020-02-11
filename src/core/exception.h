#pragma once

#include <exception>
#include <string>

namespace cq {
    // cq 命名空间下所有异常类型的基类
    struct Exception : std::exception {
        Exception(const char *what_arg) : Exception(std::string(what_arg)) {
        }

        Exception(const std::string &what_arg) : _what_arg(what_arg) {
        }

        // 异常描述
        const char *what() const noexcept override {
            return _what_arg.c_str();
        }

    protected:
        std::string _what_arg;
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
