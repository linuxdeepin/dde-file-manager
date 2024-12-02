cmake_minimum_required(VERSION 3.10)

project(dfmplugin-search)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

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

find_package(Dtk COMPONENTS Widget REQUIRED)
find_package(PkgConfig REQUIRED)

#full-text search dependences
pkg_check_modules(Lucene REQUIRED IMPORTED_TARGET liblucene++ liblucene++-contrib)
pkg_check_modules(Docparser REQUIRED IMPORTED_TARGET docparser)
pkg_check_modules(GLIB REQUIRED glib-2.0)
pkg_check_modules(PCRE REQUIRED libpcre)

# use TextIndex interface
qt5_add_dbus_interface(SRC_FILES
    "${CMAKE_SOURCE_DIR}/assets/dbus/org.deepin.Filemanager.TextIndex.xml"
    textindex_interface
)

add_library(${PROJECT_NAME}
    SHARED
    ${SRC_FILES}
)

set_target_properties(${PROJECT_NAME} PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${DFM_BUILD_PLUGIN_FILEMANAGER_DIR})

target_include_directories(${PROJECT_NAME}
    PUBLIC
        ${DtkWidget_INCLUDE_DIRS}
        ${CMAKE_SOURCE_DIR}/3rdparty
        ${GLIB_INCLUDE_DIRS}
        ${PCRE_INCLUDE_DIRS}
)

target_link_libraries(${PROJECT_NAME}
    DFM::base
    DFM::framework
    ${DtkWidget_LIBRARIES}
    ${GLIB_LIBRARIES}
    ${PCRE_LIBRARIES}
    PkgConfig::Lucene
    PkgConfig::Docparser
)

#install library file
install(TARGETS
    ${PROJECT_NAME}
    LIBRARY
    DESTINATION
    ${DFM_PLUGIN_FILEMANAGER_CORE_DIR}
)

# install dconfig files.
INSTALL_DCONFIG("org.deepin.dde.file-manager.search.json")
