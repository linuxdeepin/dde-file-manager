cmake_minimum_required(VERSION 3.1.0)

project(dfmservice-filemanager)

if (NOT VERSION)
    set(VERSION "1.0.0")
endif()

if (NOT PROJECT_VERSION_MAJOR)
    set(PROJECT_VERSION_MAJOR 0)
endif()

# windows service sources
FILE(GLOB GLOBAL_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
)

FILE(GLOB WINDOWS_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/windows/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/windows/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/windows/private/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/windows/private/*.cpp"
)


FILE(GLOB VAULT_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/vault/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/vault/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/vault/private/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/vault/private/*.cpp"
)

FILE(GLOB SIDEBAR_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/sidebar/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/sidebar/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/sidebar/private/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/sidebar/private/*.cpp"
)

FILE(GLOB TITLEBAR_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/titlebar/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/titlebar/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/titlebar/private/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/titlebar/private/*.cpp"
)

FILE(GLOB WORKSPACE_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/workspace/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/workspace/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/workspace/private/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/workspace/private/*.cpp"
)

FILE(GLOB DETAILSPACE_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/detailspace/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/detailspace/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/detailspace/private/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/detailspace/private/*.cpp"
)

FILE(GLOB SEARCH_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/search/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/search/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/search/private/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/search/fulltextsearch/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/search/fulltextsearch/*.cpp"
)

add_library(
    ${PROJECT_NAME}
    SHARED
    ${GLOBAL_FILES}
    ${WINDOWS_FILES}
    ${VAULT_FILES}
    ${SIDEBAR_FILES}
    ${TITLEBAR_FILES}
    ${WORKSPACE_FILES}
    ${DETAILSPACE_FILES}
    ${SEARCH_FILES}
)

find_package(Dtk REQUIRED Widget REQUIRED)
find_package(Qt5 COMPONENTS X11Extras REQUIRED)
find_package(PkgConfig REQUIRED)

#full-text search dependences
pkg_check_modules(Lucene REQUIRED IMPORTED_TARGET liblucene++ liblucene++-contrib)
pkg_check_modules(Docparser REQUIRED IMPORTED_TARGET docparser)

add_library(DFMSERVICE::filemanager ALIAS ${PROJECT_NAME})

set_target_properties(${PROJECT_NAME} PROPERTIES
    VERSION ${VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
)

target_include_directories(${PROJECT_NAME}
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/windows
)

target_link_libraries(${PROJECT_NAME}
    DFM::base
    DFM::framework
    DFMSERVICE::common
    Qt5::X11Extras
    ${DtkWidget_LIBRARIES}
    PkgConfig::Lucene
    PkgConfig::Docparser
)

#install library file
install(TARGETS
    ${PROJECT_NAME}
    LIBRARY
    DESTINATION
    ${LIB_INSTALL_DIR}
)

# install head file
install(DIRECTORY
    ${CMAKE_CURRENT_SOURCE_DIR}/
    DESTINATION include/dfmservice-filemanager
    FILES_MATCHING PATTERN "*.h"
    PATTERN "private" EXCLUDE
)
