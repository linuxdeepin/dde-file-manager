cmake_minimum_required(VERSION 3.10)

project(dfmplugin-disk-encrypt LANGUAGES CXX)
set(BIN_NAME dfm-disk-encrypt-plugin)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Include DFM plugin configuration module
include(DFMPluginConfig)

# 源文件
file(GLOB_RECURSE SRC
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.json"
    "${CMAKE_SOURCE_DIR}/src/services/diskencrypt/globaltypesdefine.h"
)

add_library(${BIN_NAME} SHARED ${SRC})

set_target_properties(${BIN_NAME} PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${DFM_BUILD_PLUGIN_FILEMANAGER_DIR})

# Use DFM plugin configuration system
include(${CMAKE_CURRENT_SOURCE_DIR}/dependencies.cmake)
dfm_setup_disk_encrypt_entry_dependencies(${BIN_NAME})

# 设置翻译文件目录
set(TS_DIR "${CMAKE_SOURCE_DIR}/translations/disk-encrypt-plugin")
file(GLOB TS_FILES "${TS_DIR}/disk-encrypt*.ts")

# 直接使用qt6_add_translation并指定输出路径
find_package(Qt6 REQUIRED LinguistTools)
set_source_files_properties(${TS_FILES} PROPERTIES OUTPUT_LOCATION "${CMAKE_CURRENT_BINARY_DIR}")
qt6_add_translation(QM_FILES ${TS_FILES})

# 添加自定义目标以确保翻译文件被构建
add_custom_target(${BIN_NAME}_translations ALL DEPENDS ${QM_FILES})
add_dependencies(${BIN_NAME} ${BIN_NAME}_translations)

# 安装
install(TARGETS ${BIN_NAME} LIBRARY DESTINATION ${DFM_PLUGIN_FILEMANAGER_CORE_DIR})

# 安装翻译文件
install(FILES ${QM_FILES} DESTINATION share/dde-file-manager/translations)

INSTALL_DCONFIG("org.deepin.dde.file-manager.diskencrypt.json")

message(">>>> install ${BIN_NAME} into ${DFM_PLUGIN_FILEMANAGER_CORE_DIR}")

