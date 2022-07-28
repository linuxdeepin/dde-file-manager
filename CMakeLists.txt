cmake_minimum_required(VERSION 3.10)

project(filemanager)
set(CMAKE_CXX_STANDARD 17)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall -Wl,--as-need -fPIE")

find_package(Qt5 COMPONENTS Widgets REQUIRED)
#if no debug, can't out in code define key '__FUNCTION__' and so on
add_definitions(-DQT_MESSAGELOGCONTEXT)

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX /usr)
endif()

include(GNUInstallDirs)
# defines plugins install dir
if(NOT DEFINED LIB_INSTALL_DIR)
    set(LIB_INSTALL_DIR ${CMAKE_INSTALL_FULL_LIBDIR})
endif()

# root plguins dir
if(NOT DEFINED DFM_PLUGIN_DIR)
    set(DFM_PLUGIN_DIR ${LIB_INSTALL_DIR}/dde-file-manager/plugins)
endif()

# common-core plugins dir
if(NOT DEFINED DFM_PLUGIN_COMMON_CORE_DIR)
    set(DFM_PLUGIN_COMMON_CORE_DIR ${DFM_PLUGIN_DIR}/common-core)
endif()

# desktop-core plugins dir
if(NOT DEFINED DFM_PLUGIN_DESKTOP_CORE_DIR)
    set(DFM_PLUGIN_DESKTOP_CORE_DIR ${DFM_PLUGIN_DIR}/desktop-core)
endif()

# filemanager-core plugins dir
if(NOT DEFINED DFM_PLUGIN_FILEMANAGER_CORE_DIR)
    set(DFM_PLUGIN_FILEMANAGER_CORE_DIR ${DFM_PLUGIN_DIR}/filemanager-core)
endif()

# common-edge plugins dir
if(NOT DEFINED DFM_PLUGIN_COMMON_EDGE_DIR)
    set(DFM_PLUGIN_COMMON_EDGE_DIR ${DFM_PLUGIN_DIR}/common-edge)
endif()

# daemon-edge plugins dir
if(NOT DEFINED DFM_PLUGIN_DAEMON_EDGE_DIR)
    set(DFM_PLUGIN_DAEMON_EDGE_DIR ${DFM_PLUGIN_DIR}/daemon-edge)
endif()

# desktop-edge plugins dir
if(NOT DEFINED DFM_PLUGIN_DESKTOP_EDGE_DIR)
    set(DFM_PLUGIN_DESKTOP_EDGE_DIR ${DFM_PLUGIN_DIR}/desktop-edge)
endif()

# filemanager-edge plugins dir
if(NOT DEFINED DFM_PLUGIN_FILEMANAGER_EDGE_DIR)
    set(DFM_PLUGIN_FILEMANAGER_EDGE_DIR ${DFM_PLUGIN_DIR}/filemanager-edge)
endif()

# translate
if (NOT (${CMAKE_BUILD_TYPE} MATCHES "Debug"))
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Ofast")

    # generate qm
    execute_process(COMMAND bash "translate_generation.sh"
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/src)
endif()

# sub directories
add_subdirectory(src/apps)
add_subdirectory(src/dde-dock-plugins)
add_subdirectory(src/dfm-base)
add_subdirectory(src/dfm-extension)
add_subdirectory(src/dfm-framework)
add_subdirectory(src/plugins)

option(ENABLE_UNIT_TESTS "Enable unit tests" ON)
message(STATUS "Enable testing: ${ENABLE_UNIT_TESTS}")
if(ENABLE_UNIT_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
