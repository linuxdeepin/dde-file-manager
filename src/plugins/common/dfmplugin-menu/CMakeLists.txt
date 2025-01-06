cmake_minimum_required(VERSION 3.10)

project(dfmplugin-menu)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

# 设置二进制文件名
set(BIN_NAME dfm-menu-plugin)

FILE(GLOB_RECURSE MENU_FILES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.json"
)

# Qt6 依赖
find_package(Qt6 COMPONENTS Core REQUIRED)

# 创建共享库
add_library(${BIN_NAME}
    SHARED
    ${MENU_FILES}
)

set_target_properties(${BIN_NAME} PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${DFM_BUILD_PLUGIN_COMMON_DIR})

# 链接必要的库
target_link_libraries(${BIN_NAME}
    DFM6::base
    DFM6::framework
)

# 安装库文件
install(TARGETS
    ${BIN_NAME}
    LIBRARY
    DESTINATION
    ${DFM_PLUGIN_COMMON_CORE_DIR}
)
