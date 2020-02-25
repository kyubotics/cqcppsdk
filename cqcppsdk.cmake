# 引用 CMakeLists.txt
add_subdirectory(${CMAKE_CURRENT_LIST_DIR})
get_directory_property(CQ_CAN_BUILD_STD_MODE DIRECTORY ${CMAKE_CURRENT_LIST_DIR} DEFINITION CQ_CAN_BUILD_STD_MODE)

# 设置 app id
macro(cq_set_app_id APP_ID)
    message(STATUS "app id: ${APP_ID}")
    set(_CQ_APP_ID ${APP_ID})
    add_definitions(-D_CQ_APP_ID="${APP_ID}")
endmacro()

# 添加酷Q应用构建目标, 在调用之前设置 CQCPPSDK_DEV_MODE 为 ON 可构建 dev 模式的可执行文件
function(cq_add_app OUT_NAME)
    if (CQCPPSDK_DEV_MODE)
        # dev 模式, 产生可执行文件, 用于脱离酷Q单独测试业务逻辑, 不限平台
        message(STATUS "add dev mode executable: ${OUT_NAME}")
        add_executable(${OUT_NAME} ${ARGN})
        target_link_libraries(${OUT_NAME} cqcppsdk_dev_mode)
    elseif (NOT CQ_CAN_BUILD_STD_MODE)
        # 试图构建 std 模式, 但当前工具链无法构建
        message(SEND_ERROR "can not build std mode dll with the current toolchain")
    else ()
        # std 模式, 产生 Windows 动态链接库, 即可被酷Q加载的应用(进而可打包为 CPK)
        if (NOT OUT_NAME STREQUAL "app")
            message(WARNING "the name of std mode dll is required to be \"app\"")
        endif ()
        message(STATUS "add std mode dll: ${OUT_NAME}")
        add_library(${OUT_NAME} MODULE ${ARGN})
        target_link_libraries(${OUT_NAME} cqcppsdk_std_mode)
        set_target_properties(${OUT_NAME} PROPERTIES PREFIX "") # 去除 lib 前缀
    endif ()
endfunction()

# 添加 std 模式构建目标, 可选参数为源文件列表
function(cq_add_std_app)
    set(CQCPPSDK_DEV_MODE OFF)
    cq_add_app(app ${ARGN})
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
