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

# 设置 app id, 必须在 CMakeLists.txt 中调用, 否则无法编译
macro(cq_set_app_id APP_ID)
    message(STATUS "app id: ${APP_ID}")
    set(_CQ_APP_ID ${APP_ID})
    add_definitions(-DAPP_ID="${APP_ID}")
endmacro()

# 添加酷Q应用构建目标, 在调用之前设置 CQCPPSDK_DEV_MODE 为 ON 可构建 dev 模式的可执行文件
function(cq_add_app OUT_NAME)
    message(STATUS "dev mode: ${CQCPPSDK_DEV_MODE}")

    if (CQCPPSDK_DEV_MODE)
        # dev 模式, 产生可执行文件, 用于脱离酷Q单独测试业务逻辑, 不限平台
        add_definitions(-D_CQ_DEV_MODE)
        file(GLOB_RECURSE _CQCPPSDK_MODE_SOURCE_FILES ${_CQCPPSDK_DIR}/src/dev_mode/*.cpp)
        message(STATUS "add dev mode executable: ${OUT_NAME}")
        add_executable(${OUT_NAME} ${ARGN} ${_CQCPPSDK_SOURCE_FILES} ${_CQCPPSDK_MODE_SOURCE_FILES})
    elseif (NOT CQ_CAN_BUILD_STD_MODE)
        # 试图构建 std 模式, 但当前工具链无法构建
        message(FATAL_ERROR "can not build std mode dll with the current toolchain")
    else ()
        # std 模式, 产生 Windows 动态链接库, 即可被酷Q加载的插件(进而可打包为 CPK)
        if (NOT OUT_NAME STREQUAL "app")
            message(WARNING "the name of std mode dll is required to be \"app\"")
        endif ()
        add_definitions(-D_CQ_STD_MODE)
        file(GLOB_RECURSE _CQCPPSDK_MODE_SOURCE_FILES ${_CQCPPSDK_DIR}/src/std_mode/*.cpp)
        message(STATUS "add std mode dll: ${OUT_NAME}")
        add_library(${OUT_NAME} SHARED ${ARGN} ${_CQCPPSDK_SOURCE_FILES} ${_CQCPPSDK_MODE_SOURCE_FILES})
        set_target_properties(${OUT_NAME} PROPERTIES PREFIX "") # 去除 lib 前缀
    endif ()

    target_compile_options(${OUT_NAME} PRIVATE $<$<CXX_COMPILER_ID:MSVC>:/utf-8>)

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

# 添加 std 模式构建目标, 可选参数为源文件列表
function(cq_add_std_app)
    if (CQ_CAN_BUILD_STD_MODE)
        set(CQCPPSDK_DEV_MODE OFF)
        cq_add_app(app ${ARGN})
    else ()
        message(WARNING "can not build std mode dll with the current toolchain")
    endif ()
endfunction()

# 添加 dev 模式构建目标, 可选参数为源文件列表
function(cq_add_dev_app)
    set(CQCPPSDK_DEV_MODE ON)
    cq_add_app(app_dev ${ARGN})
endfunction()

# 已弃用
# 添加安装脚本, 将在指定构建目标构建完成后运行, 可用于拷贝 app.dll 和 app.json 到酷Q应用目录
function(cq_add_install_script OUT_NAME INSTALL_SCRIPT)
    message(DEPRECATION "cq_add_install_script is deprecated, please use cq_install_std_app instead")
    if (EXISTS ${INSTALL_SCRIPT})
        add_custom_command(TARGET ${OUT_NAME}
                POST_BUILD
                COMMAND
                powershell -ExecutionPolicy Bypass -NoProfile -File ${INSTALL_SCRIPT} "$<TARGET_FILE_DIR:${OUT_NAME}>")
    endif ()
endfunction()

# 安装 std 模式的酷Q应用, 可选参数为酷Q主目录位置列表, 如不传入参数则使用 CMAKE_INSTALL_PREFIX
function(cq_install_std_app)
    if (NOT TARGET app)
        message(WARNING "std mode target is not defined")
        return()
    endif ()

    macro(_cq_install_std_app PREFIX)
        if (PREFIX AND (NOT PREFIX MATCHES ".*[/\\]$"))
            set(DESTDIR "${PREFIX}/dev/${_CQ_APP_ID}")
        else ()
            set(DESTDIR "${PREFIX}dev/${_CQ_APP_ID}")
        endif ()
        message(STATUS "install destination: ${DESTDIR}")
        install(FILES
                "$<TARGET_FILE_DIR:app>/app.dll"
                "${CMAKE_SOURCE_DIR}/app.json"
                DESTINATION ${DESTDIR})
    endmacro()

    if (ARGC GREATER 0)
        # 如果传入了参数, 则安装到参数指定的酷Q位置
        foreach (PREFIX IN LISTS ARGN)
            _cq_install_std_app(${PREFIX})
        endforeach ()
    else ()
        # 否则安装到 CMAKE_INSTALL_PREFIX 指定的位置
        _cq_install_std_app("")
    endif ()
endfunction()
