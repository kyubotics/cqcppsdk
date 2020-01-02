#pragma once

#if defined(_CQ_STD_MODE)
#define __CQ_EVENT(ReturnType, FuncName, ParamsSize)                                                                \
    __pragma(comment(linker, "/EXPORT:" #FuncName "=_" #FuncName "@" #ParamsSize)) extern "C" __declspec(dllexport) \
        ReturnType __stdcall FuncName
#else
#define __CQ_EVENT(ReturnType, FuncName, ParamsSize) ReturnType FuncName
#endif
