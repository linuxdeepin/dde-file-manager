cmake_minimum_required(VERSION 3.10)

project(dfmplugin-tag)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

# 设置二进制文件名
set(BIN_NAME dfm-tag-plugin)

FILE(GLOB_RECURSE TAG_FILES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.json"
)

# Qt6 依赖
find_package(Qt6 COMPONENTS Core DBus REQUIRED)

# 生成 DBus 接口
qt6_add_dbus_interface(Qt6App_dbus
    ${DFM_DBUS_XML_DIR}/org.deepin.Filemanager.Daemon.TagManager.xml
    tagmanager_interface_qt6)

# 创建共享库
add_library(${BIN_NAME}
    SHARED
    ${TAG_FILES}
    ${Qt6App_dbus}
)

set_target_properties(${BIN_NAME} PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${DFM_BUILD_PLUGIN_COMMON_DIR})

# 链接必要的库
target_link_libraries(${BIN_NAME}
    Qt6::DBus
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
