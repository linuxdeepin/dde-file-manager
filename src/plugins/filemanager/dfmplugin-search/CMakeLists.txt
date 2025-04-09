cmake_minimum_required(VERSION 3.10)

project(dfmplugin-search)
set(BIN_NAME dfm-search-plugin)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(Qt6 REQUIRED COMPONENTS DBus)
find_package(dfm6-search REQUIRED)

FILE(GLOB_RECURSE SRC_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_SOURCE_DIR}/3rdparty/fsearch/*.c"
    "${CMAKE_SOURCE_DIR}/3rdparty/fsearch/*.h"
    "${CMAKE_SOURCE_DIR}/3rdparty/fulltext/*.cpp"
    "${CMAKE_SOURCE_DIR}/3rdparty/fulltext/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.json"
)

if (${CMAKE_SYSTEM_PROCESSOR} MATCHES "aarch64")
    ADD_DEFINITIONS(-DARM_PROCESSOR)
endif()

find_package(PkgConfig REQUIRED)

#full-text search dependences
pkg_check_modules(Lucene REQUIRED IMPORTED_TARGET liblucene++ liblucene++-contrib)
pkg_check_modules(Docparser REQUIRED IMPORTED_TARGET docparser)
pkg_check_modules(GLIB REQUIRED glib-2.0)
pkg_check_modules(PCRE REQUIRED libpcre)

# use TextIndex interface
set(DBUS_INTERFACE_FILE "${CMAKE_SOURCE_DIR}/assets/dbus/org.deepin.Filemanager.TextIndex.xml")
qt_add_dbus_interface(INTERFACE_SOURCES ${DBUS_INTERFACE_FILE} textindex_interface)

add_library(${BIN_NAME}
    SHARED
    ${SRC_FILES}
    ${INTERFACE_SOURCES}
)

set_target_properties(${BIN_NAME} PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${DFM_BUILD_PLUGIN_FILEMANAGER_DIR})

target_include_directories(${BIN_NAME}
    PUBLIC
        ${CMAKE_SOURCE_DIR}/3rdparty
        ${GLIB_INCLUDE_DIRS}
        ${PCRE_INCLUDE_DIRS}
        ${dfm6-search_INCLUDE_DIR}
)

target_link_libraries(${BIN_NAME}
    DFM6::base
    DFM6::framework
    ${GLIB_LIBRARIES}
    ${PCRE_LIBRARIES}
    PkgConfig::Lucene
    PkgConfig::Docparser
    dfm6-search
)

#install library file
install(TARGETS
    ${BIN_NAME}
    LIBRARY
    DESTINATION
    ${DFM_PLUGIN_FILEMANAGER_CORE_DIR}
)

# install dconfig files.
INSTALL_DCONFIG("org.deepin.dde.file-manager.search.json")
