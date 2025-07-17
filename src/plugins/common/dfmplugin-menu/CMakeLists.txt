cmake_minimum_required(VERSION 3.10)

project(dfmplugin-menu)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Include plugin configuration
include(DFMPluginConfig)

# 设置二进制文件名
set(BIN_NAME dfm-menu-plugin)

FILE(GLOB_RECURSE MENU_FILES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.json"
)

# 创建共享库
add_library(${BIN_NAME}
    SHARED
    ${MENU_FILES}
)

set_target_properties(${BIN_NAME} PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${DFM_BUILD_PLUGIN_COMMON_DIR})

# Use default plugin configuration
dfm_apply_default_plugin_config(${BIN_NAME})

# Add menu-specific compile definitions
target_compile_definitions(${BIN_NAME} PRIVATE MENU_CHECK_FOCUSONLY)

# 安装库文件
install(TARGETS
    ${BIN_NAME}
    LIBRARY
    DESTINATION
    ${DFM_PLUGIN_COMMON_CORE_DIR}
)
