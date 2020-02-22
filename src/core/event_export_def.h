#pragma once

#if defined(_CQ_STD_MODE)

#if defined(_MSC_VER)
#define _CQ_EVENT(ReturnType, FuncName, ParamsSize)                                                                 \
    __pragma(comment(linker, "/EXPORT:" #FuncName "=_" #FuncName "@" #ParamsSize)) extern "C" __declspec(dllexport) \
        ReturnType __stdcall FuncName
#else
#define _CQ_EVENT(ReturnType, FuncName, ParamsSize) extern "C" __declspec(dllexport) ReturnType __stdcall FuncName
#endif // defined(_MSC_VER)

#else
#define _CQ_EVENT(ReturnType, FuncName, ParamsSize) ReturnType FuncName
#endif // defined(_CQ_STD_MODE)
