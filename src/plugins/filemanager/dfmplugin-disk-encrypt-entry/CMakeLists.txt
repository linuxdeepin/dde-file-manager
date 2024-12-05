cmake_minimum_required(VERSION 3.0)

project(dfmplugin-disk-encrypt LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(Qt5 REQUIRED COMPONENTS Core Widgets Concurrent)

file(GLOB_RECURSE SRC
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.json"
    "${CMAKE_SOURCE_DIR}/src/services/diskencrypt/globaltypesdefine.h"
)

add_library(${PROJECT_NAME} SHARED
  ${SRC}
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    DFM${DTK_VERSION_MAJOR}::base
    DFM${DTK_VERSION_MAJOR}::framework
)

target_compile_definitions(${PROJECT_NAME} PRIVATE DFMPLUGIN_DISK_ENCRYPT_LIBRARY)
set_target_properties(${PROJECT_NAME} PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${DFM_BUILD_PLUGIN_FILEMANAGER_DIR})

install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION ${DFM_PLUGIN_FILEMANAGER_CORE_DIR})
message(">>>> install ${PROJECT_NAME} into ${DFM_PLUGIN_FILEMANAGER_CORE_DIR}")

execute_process(
    COMMAND lupdate
        ./
        -ts
        -no-obsolete
        ${CMAKE_SOURCE_DIR}/translations/disk-encrypt-plugin/disk-encrypt.ts
    COMMAND lupdate
        ./
        -ts
        -no-obsolete
        ${CMAKE_SOURCE_DIR}/translations/disk-encrypt-plugin/disk-encrypt_zh_CN.ts

    WORKING_DIRECTORY
        ${CMAKE_CURRENT_SOURCE_DIR}
)

# 查找匹配 disk-encrypt*.ts 的文件列表
file(GLOB TS_FILES ${CMAKE_SOURCE_DIR}/translations/disk-encrypt-plugin/disk-encrypt*.ts)
file(GLOB QM_FILES ${CMAKE_SOURCE_DIR}/translations/disk-encrypt-plugin/disk-encrypt*.qm)

# 添加 lrelease 命令，传递 TS_FILES 列表
foreach(TS_FILE ${TS_FILES})
    execute_process(
       COMMAND lrelease ${TS_FILE}
       WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    )
endforeach()

install(FILES ${QM_FILES} DESTINATION share/dde-file-manager/translations)
INSTALL_DCONFIG("org.deepin.dde.file-manager.diskencrypt.json")

