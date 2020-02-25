cmake_minimum_required(VERSION 3.9)
project(cqcppsdk)

set(_CQ_APP_ID CACHE STRING FORCE)
option(_CQ_BUILD_TESTS "Build CQCPPSDK tests" OFF)

if (WIN32)
    add_definitions(-DWIN32) # 确保 Win32 环境下存在 WIN32 定义
endif ()

# 检查是否支持构建 std 模式
if ((MSVC OR MSYS OR MINGW) AND (CMAKE_SIZEOF_VOID_P EQUAL 4))
    set(CQ_CAN_BUILD_STD_MODE YES)
endif ()

# 获取 SDK 所在路径
set(_CQCPPSDK_DIR ${CMAKE_CURRENT_LIST_DIR})
message(STATUS "cqcppsdk dir: ${_CQCPPSDK_DIR}")

# 递归搜索 core 和 utils 中的源文件
file(GLOB_RECURSE _CQCPPSDK_SOURCE_FILES ${_CQCPPSDK_DIR}/src/core/*.cpp ${_CQCPPSDK_DIR}/src/utils/*.cpp)

# 添加酷QSDK项目
add_library(cqcppsdk INTERFACE)
target_sources(cqcppsdk INTERFACE ${_CQCPPSDK_SOURCE_FILES})
target_include_directories(cqcppsdk INTERFACE ${_CQCPPSDK_DIR}/include)
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    # 在 GCC 8.x 环境下使用 std::filesystem 需要链接 stdc++fs
    target_link_libraries(cqcppsdk INTERFACE stdc++fs)
endif ()

if(CQ_CAN_BUILD_STD_MODE)
    add_library(cqcppsdk_std INTERFACE)
    file(GLOB_RECURSE _CQCPPSDK_STD_MODE_SOURCE_FILES ${_CQCPPSDK_DIR}/src/std_mode/*.cpp)
    target_sources(cqcppsdk_std INTERFACE ${_CQCPPSDK_DEV_MODE_SOURCE_FILES})
    target_compile_definitions(cqcppsdk_std INTERFACE -D_CQ_STD_MODE -DAPP_ID="${_CQ_APP_ID}")
    target_link_libraries(cqcppsdk_std INTERFACE cqcppsdk)

    if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        target_link_options(cqcppsdk_std INTERFACE -static -Wl,--kill-at,--enable-stdcall-fixup)
    endif()
endif()

add_library(cqcppsdk_dev INTERFACE)
file(GLOB_RECURSE _CQCPPSDK_DEV_MODE_SOURCE_FILES ${_CQCPPSDK_DIR}/src/dev_mode/*.cpp)
target_sources(cqcppsdk_dev INTERFACE ${_CQCPPSDK_DEV_MODE_SOURCE_FILES})
target_compile_definitions(cqcppsdk_dev INTERFACE -D_CQ_DEV_MODE)
target_link_libraries(cqcppsdk_dev INTERFACE cqcppsdk)

if(_CQ_BUILD_TESTS)
    add_subdirectory(tests)
endif()