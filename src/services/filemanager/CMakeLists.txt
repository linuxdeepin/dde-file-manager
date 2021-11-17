cmake_minimum_required(VERSION 3.1.0)

project(dfmservice-filemanager)

# window service sources

FILE(GLOB GLOBAL_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    )

FILE(GLOB WINDOW_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/window/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/window/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/window/private/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/window/private/*.cpp"
    )

FILE(GLOB SHORTCUT_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/shortcut/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/shortcut/*.cpp"
    )

FILE(GLOB VAULT_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/vault/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/vault/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/vault/private/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/vault/private/*.cpp"
    )

add_library(
    ${PROJECT_NAME}
    SHARED
    ${GLOBAL_FILES}
    ${WINDOW_FILES}
    ${SHORTCUT_FILES}
    ${VAULT_FILES}
    )

target_link_libraries(
    ${PROJECT_NAME}
    DFM::base
    DFM::framework
    ${QtUseModules}
    ${PkgUseModules}
    )

#install library file
install(TARGETS
    ${PROJECT_NAME}
    LIBRARY
    DESTINATION
    ${LIB_INSTALL_DIR}
    )