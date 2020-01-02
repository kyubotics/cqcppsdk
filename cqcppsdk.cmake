if(MSVC)
    set(CMAKE_CXX_FLAGS "/utf-8 ${CMAKE_CXX_FLAGS}") # 设置 MSVC 编译器识别 UTF-8 编码的源文件
endif()

if(WIN32)
    add_definitions(-DWIN32)
endif()

set(_CQCPPSDK_DIR ${CMAKE_CURRENT_LIST_DIR})
message(STATUS "cqcppsdk is in ${_CQCPPSDK_DIR}")

file(GLOB_RECURSE _CQCPPSDK_SOURCE_FILES ${_CQCPPSDK_DIR}/src/core/*.cpp ${_CQCPPSDK_DIR}/src/utils/*.cpp)

# 设置 App Id, 必须在 CMakeLists.txt 中调用, 否则无法编译
function(set_cq_app_id APP_ID)
    message(STATUS "app id: ${APP_ID}")
    add_definitions(-DAPP_ID="${APP_ID}")
endfunction()

# 添加酷Q应用构建目标
function(add_cq_app OUT_NAME SOURCE_FILES)
    message(STATUS "dev mode: ${CQCPPSDK_DEV_MODE}")
    if(CQCPPSDK_DEV_MODE)
        # dev 模式，产生可执行文件，用于脱离酷Q单独测试业务逻辑，不限平台
        add_definitions(-D_CQ_DEV_MODE)
        file(GLOB_RECURSE _CQCPPSDK_MODE_SOURCE_FILES ${_CQCPPSDK_DIR}/src/dev_mode/*.cpp)
        add_executable(${OUT_NAME} ${SOURCE_FILES} ${_CQCPPSDK_SOURCE_FILES} ${_CQCPPSDK_MODE_SOURCE_FILES})
    else()
        # std 模式，产生 Windows 动态链接库，即可被酷Q加载的插件（进而可打包为 CPK），仅限 Windows 上使用 MSVC x86 工具链构建
        add_definitions(-D_CQ_STD_MODE)
        file(GLOB_RECURSE _CQCPPSDK_MODE_SOURCE_FILES ${_CQCPPSDK_DIR}/src/std_mode/*.cpp)
        add_library(${OUT_NAME} SHARED ${SOURCE_FILES} ${_CQCPPSDK_SOURCE_FILES} ${_CQCPPSDK_MODE_SOURCE_FILES})
    endif()
endfunction()
