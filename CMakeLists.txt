cmake_minimum_required(VERSION 3.9)
project(cqcppsdk)

option(CQ_BUILD_TESTS "Build CQCPPSDK tests" OFF)
message(STATUS "build cqcppsdk tests: ${CQ_BUILD_TESTS}")

if (WIN32)
    add_definitions(-DWIN32) # 确保 Win32 环境下存在 WIN32 定义
endif ()

# 检查是否支持构建 std 模式
if (WIN32 AND (MSVC OR MSYS OR MINGW) AND (CMAKE_SIZEOF_VOID_P EQUAL 4))
    set(CQ_CAN_BUILD_STD_MODE YES)
else ()
    set(CQ_CAN_BUILD_STD_MODE NO)
endif ()
message(STATUS "can build std mode: ${CQ_CAN_BUILD_STD_MODE}")

# 获取 SDK 所在目录
set(_CQCPPSDK_DIR ${CMAKE_CURRENT_LIST_DIR})
message(STATUS "cqcppsdk dir: ${_CQCPPSDK_DIR}")

# 递归搜索 core 和 utils 中的源文件
file(GLOB_RECURSE _CQCPPSDK_SOURCE_FILES ${_CQCPPSDK_DIR}/src/core/*.cpp ${_CQCPPSDK_DIR}/src/utils/*.cpp)

# 添加 SDK 核心部分
add_library(cqcppsdk INTERFACE)
target_sources(cqcppsdk INTERFACE ${_CQCPPSDK_SOURCE_FILES})
target_include_directories(cqcppsdk INTERFACE ${_CQCPPSDK_DIR}/include)
target_compile_options(cqcppsdk INTERFACE $<$<CXX_COMPILER_ID:MSVC>:/utf-8>)
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 9.0)
    # 在 GCC 8.x 环境下使用 std::filesystem 需要链接 stdc++fs
    target_link_libraries(cqcppsdk INTERFACE stdc++fs)
endif ()

if (CQ_CAN_BUILD_STD_MODE)
    # 添加 std 模式
    add_library(cqcppsdk_std_mode INTERFACE)
    file(GLOB_RECURSE _CQCPPSDK_STD_MODE_SOURCE_FILES ${_CQCPPSDK_DIR}/src/std_mode/*.cpp)
    target_sources(cqcppsdk_std_mode INTERFACE ${_CQCPPSDK_STD_MODE_SOURCE_FILES} ${_CQCPPSDK_DIR}/src/api_test.cpp)
    target_compile_definitions(cqcppsdk_std_mode INTERFACE -D_CQ_STD_MODE)
    target_link_libraries(cqcppsdk_std_mode INTERFACE cqcppsdk)
    if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        target_link_options(cqcppsdk_std_mode INTERFACE -static -Wl,--kill-at,--enable-stdcall-fixup)
    endif ()
endif ()

# 添加 dev 模式
add_library(cqcppsdk_dev_mode INTERFACE)
file(GLOB_RECURSE _CQCPPSDK_DEV_MODE_SOURCE_FILES ${_CQCPPSDK_DIR}/src/dev_mode/*.cpp)
target_sources(cqcppsdk_dev_mode INTERFACE ${_CQCPPSDK_DEV_MODE_SOURCE_FILES} ${_CQCPPSDK_DIR}/src/api_test.cpp)
target_compile_definitions(cqcppsdk_dev_mode INTERFACE -D_CQ_DEV_MODE)
target_link_libraries(cqcppsdk_dev_mode INTERFACE cqcppsdk)

if (CQ_BUILD_TESTS)
    # 添加 test 模式
    add_library(cqcppsdk_test_mode INTERFACE)
    target_compile_definitions(cqcppsdk_test_mode INTERFACE -D_CQ_TEST_MODE)
    target_link_libraries(cqcppsdk_test_mode INTERFACE cqcppsdk)
    add_subdirectory(tests)
endif ()
