cmake_minimum_required(VERSION 3.0)

project(dfmplugin-disk-encrypt LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(Qt5 REQUIRED COMPONENTS Core Widgets Concurrent LinguistTools)

# 源文件
file(GLOB_RECURSE SRC
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.json"
    "${CMAKE_SOURCE_DIR}/src/services/diskencrypt/globaltypesdefine.h"
)

add_library(${PROJECT_NAME} SHARED ${SRC})

target_link_libraries(${PROJECT_NAME} PRIVATE
    DFM${DTK_VERSION_MAJOR}::base
    DFM${DTK_VERSION_MAJOR}::framework
)

target_compile_definitions(${PROJECT_NAME} PRIVATE DFMPLUGIN_DISK_ENCRYPT_LIBRARY)
set_target_properties(${PROJECT_NAME} PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${DFM_BUILD_PLUGIN_FILEMANAGER_DIR})

# 设置翻译文件目录
set(TS_DIR "${CMAKE_SOURCE_DIR}/translations/disk-encrypt-plugin")
file(GLOB TS_FILES "${TS_DIR}/disk-encrypt*.ts")

# 直接使用qt5_add_translation并指定输出路径
set_source_files_properties(${TS_FILES} PROPERTIES OUTPUT_LOCATION "${CMAKE_CURRENT_BINARY_DIR}")
qt5_add_translation(QM_FILES ${TS_FILES})

# 添加自定义目标以确保翻译文件被构建
add_custom_target(${PROJECT_NAME}_translations ALL DEPENDS ${QM_FILES})
add_dependencies(${PROJECT_NAME} ${PROJECT_NAME}_translations)

# 安装
install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION ${DFM_PLUGIN_FILEMANAGER_CORE_DIR})

# 安装翻译文件
install(FILES ${QM_FILES} DESTINATION share/dde-file-manager/translations)

INSTALL_DCONFIG("org.deepin.dde.file-manager.diskencrypt.json")

message(">>>> install ${PROJECT_NAME} into ${DFM_PLUGIN_FILEMANAGER_CORE_DIR}")

