cmake_minimum_required(VERSION 3.10)

project(dfmplugin-fileoperations)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

# 设置二进制文件名
set(BIN_NAME dfm-fileoperations-plugin)

FILE(GLOB_RECURSE FILEOPERATIONS_FILES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.json"
)

# 查找依赖包
find_package(PkgConfig REQUIRED)
pkg_check_modules(zlib REQUIRED zlib IMPORTED_TARGET)
find_package(Qt6 COMPONENTS Core DBus REQUIRED)

# 创建共享库
add_library(${BIN_NAME}
    SHARED
    ${FILEOPERATIONS_FILES}
)

set_target_properties(${BIN_NAME} PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${DFM_BUILD_PLUGIN_COMMON_DIR})

# 添加 DBus 接口
qt6_add_dbus_interface(Qt6App_dbus
    ${DFM_DBUS_XML_DIR}/org.deepin.Filemanager.Daemon.OperationsStackManager.xml
    operationsstackmanager_interface_qt6)
target_sources(${BIN_NAME} PRIVATE ${Qt6App_dbus})

# 链接必要的库
target_link_libraries(${BIN_NAME}
    Qt6::Core
    Qt6::DBus
    DFM6::base
    DFM6::framework
    PkgConfig::zlib
)

# 安装库文件
install(TARGETS
    ${BIN_NAME}
    LIBRARY
    DESTINATION
    ${DFM_PLUGIN_COMMON_CORE_DIR}
)

# 安装 dconfig 文件
INSTALL_DCONFIG("org.deepin.dde.file-manager.operations.json")
