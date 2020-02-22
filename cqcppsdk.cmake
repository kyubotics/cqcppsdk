if (MSVC AND NOT (${CMAKE_CXX_FLAGS} MATCHES "/[uU][tT][fF]-8"))
    message(STATUS "turn on msvc utf-8")
    set(CMAKE_CXX_FLAGS "/utf-8 ${CMAKE_CXX_FLAGS}") # 设置 MSVC 编译器识别 UTF-8 编码的源文件
endif ()

if (WIN32)
    add_definitions(-DWIN32) # 确保 Win32 环境下存在 WIN32 定义
endif ()

# 检查是否支持构建 std 模式
if ((MSVC OR MSYS OR MINGW) AND (CMAKE_SIZEOF_VOID_P EQUAL 4))
    set(CQ_CAN_BUILD_STD_MODE YES)
endif ()

# 获取 cqcppsdk 所在路径
set(_CQCPPSDK_DIR ${CMAKE_CURRENT_LIST_DIR})
message(STATUS "cqcppsdk dir: ${_CQCPPSDK_DIR}")

# 递归搜索 core 和 utils 中的源文件
file(GLOB_RECURSE _CQCPPSDK_SOURCE_FILES ${_CQCPPSDK_DIR}/src/core/*.cpp ${_CQCPPSDK_DIR}/src/utils/*.cpp)

# 设置 app id, 必须在 CMakeLists.txt 中调用, 否则无法编译
function(cq_set_app_id APP_ID)
    message(STATUS "app id: ${APP_ID}")
    add_definitions(-DAPP_ID="${APP_ID}")
endfunction()

# 添加酷Q应用构建目标, 在调用之前设置 CQCPPSDK_DEV_MODE 为 ON 可构建 dev 模式的可执行文件
function(cq_add_app OUT_NAME)
    message(STATUS "dev mode: ${CQCPPSDK_DEV_MODE}")

    if (CQCPPSDK_DEV_MODE)
        # Dev 模式, 产生可执行文件, 用于脱离酷Q单独测试业务逻辑, 不限平台
        add_definitions(-D_CQ_DEV_MODE)
        file(GLOB_RECURSE _CQCPPSDK_MODE_SOURCE_FILES ${_CQCPPSDK_DIR}/src/dev_mode/*.cpp)
        message(STATUS "add dev mode executable: ${OUT_NAME}")
        add_executable(${OUT_NAME} ${ARGN} ${_CQCPPSDK_SOURCE_FILES} ${_CQCPPSDK_MODE_SOURCE_FILES})
    elseif (NOT CQ_CAN_BUILD_STD_MODE)
        # 试图构建 std 模式, 但当前工具链无法构建
        message(SEND_ERROR "can not build std mode with the current toolchain")
    else ()
        # Std 模式, 产生 Windows 动态链接库, 即可被酷Q加载的插件(进而可打包为 CPK)
        add_definitions(-D_CQ_STD_MODE)
        file(GLOB_RECURSE _CQCPPSDK_MODE_SOURCE_FILES ${_CQCPPSDK_DIR}/src/std_mode/*.cpp)
        message(STATUS "add std mode dll: ${OUT_NAME}")
        add_library(${OUT_NAME} SHARED ${ARGN} ${_CQCPPSDK_SOURCE_FILES} ${_CQCPPSDK_MODE_SOURCE_FILES})
        set_target_properties(${OUT_NAME} PROPERTIES PREFIX "") # 去除 lib 前缀
    endif ()

    if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        # MinGW 编译时, 静态链接, 修正 stdcall 导出名, 支持导入 stdcall 的 dll (--enable-stdcall-fixup)
        if (NOT CQCPPSDK_DEV_MODE)
            target_link_options(${OUT_NAME} PRIVATE -static -Wl,--kill-at,--enable-stdcall-fixup)
        endif ()

        if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 9.0)
            # 在 GCC 8.x 环境下使用 std::filesystem 需要链接 stdc++fs
            target_link_libraries(${OUT_NAME} stdc++fs)
        endif ()
    endif ()
endfunction()

# 添加安装脚本, 将在指定构建目标构建完成后运行, 可用于拷贝 app.dll 和 app.json 到酷Q应用目录
function(cq_add_install_script OUT_NAME INSTALL_SCRIPT)
    if (EXISTS ${INSTALL_SCRIPT})
        add_custom_command(TARGET ${OUT_NAME}
                POST_BUILD
                COMMAND
                powershell -ExecutionPolicy Bypass -NoProfile -File ${INSTALL_SCRIPT} "$<TARGET_FILE_DIR:${OUT_NAME}>")
    endif ()
endfunction()
