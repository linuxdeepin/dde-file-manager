cmake_minimum_required(VERSION 3.10)

project(filemanager)
set(CMAKE_CXX_STANDARD 17)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall -Wl,--as-needed")

#安全加固
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fstack-protector-all -Werror=return-type")
if (${CMAKE_SYSTEM_PROCESSOR} MATCHES "mips")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -z noexecstack -z relro")
    add_definitions(-D ARCH_MIPSEL)
endif()

#CMake module
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(install_plugin_quick_module)
include(install_dconfig)

# Select major version.
find_package(QT NAMES Qt6 REQUIRED COMPONENTS Core)
message("Using Qt version: ${QT_VERSION_MAJOR}")

if(QT_VERSION_MAJOR EQUAL "6")
    set(DFM_BUILD_WITH_QT6 ON)
    set(DTK_VERSION_MAJOR 6)
else()
    set(DTK_VERSION_MAJOR "")
endif()

find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets REQUIRED)
#if no debug, can't out in codke define key '__FUNCTION__' and so on
add_definitions(-DQT_MESSAGELOGCONTEXT)

# TODO: control it by CI. (Automation Testing)
add_definitions(-DENABLE_TESTING)

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX /usr)
endif()

#Indentify the version
# TODO: remove it
add_definitions(-DCOMPILE_ON_V23)
set(COMPLIE_ON_V23 TRUE)
message("COMPILE ON v23")

# TODO(zhangs): enable it
# Disbale qDebug(), qInfo(), qWaring(), qCritical()
# Use qCDebug() instead of qDebug()
option(DISABLE_DEBUG_MACRO "Disable Debug Macro" Off)
if(DISABLE_DEBUG_MACRO)
    add_definitions(-DDFM_DISABLE_DEBUG_MACRO)
endif()

include(GNUInstallDirs)

# dbus xml dir
set(DFM_DBUS_XML_DIR ${CMAKE_SOURCE_DIR}/assets/dbus)

# defines plugins install dir
if(NOT DEFINED LIB_INSTALL_DIR)
    set(LIB_INSTALL_DIR ${CMAKE_INSTALL_FULL_LIBDIR})
endif()

# root plguins dir
if(NOT DEFINED DFM_PLUGIN_DIR)
    set(DFM_PLUGIN_DIR ${LIB_INSTALL_DIR}/dde-file-manager/plugins)
endif()

# build plugins dir
if(NOT DEFINED DFM_BUILD_PLUGIN_DIR)
    set(DFM_BUILD_PLUGIN_DIR ${CMAKE_CURRENT_BINARY_DIR}/src/plugins)
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

# server-edge plugins dir
if(NOT DEFINED DFM_PLUGIN_SERVER_EDGE_DIR)
    set(DFM_PLUGIN_SERVER_EDGE_DIR ${DFM_PLUGIN_DIR}/server-edge)
endif()

# desktop-edge plugins dir
if(NOT DEFINED DFM_PLUGIN_DESKTOP_EDGE_DIR)
    set(DFM_PLUGIN_DESKTOP_EDGE_DIR ${DFM_PLUGIN_DIR}/desktop-edge)
endif()

# filemanager-edge plugins dir
if(NOT DEFINED DFM_PLUGIN_FILEMANAGER_EDGE_DIR)
    set(DFM_PLUGIN_FILEMANAGER_EDGE_DIR ${DFM_PLUGIN_DIR}/filemanager-edge)
endif()

# tools
if(NOT DEFINED DFM_TOOLS_DIR)
    set(DFM_TOOLS_DIR ${LIB_INSTALL_DIR}/dde-file-manager/tools)
endif()

# dde-file-thumbnail-tool
if(NOT DEFINED DFM_THUMBNAIL_TOOL)
    set(DFM_THUMBNAIL_TOOL ${LIB_INSTALL_DIR}/dde-file-manager/tools)
endif()

# qml module
if(NOT DEFINED DFM_QML_MODULE)
    set(DFM_QML_MODULE ${DFM_PLUGIN_DIR}/qml)
endif()

# qml import path for qt creator
list(APPEND QML_DIRS ${DFM_BUILD_PLUGIN_DIR}/qml)
set(QML_IMPORT_PATH ${QML_DIRS} CACHE STRING "Import local module" FORCE)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Release" CACHE STRING
      "Choose the type of build, options are: Debug Release RelWithDebInfo MinSizeRel."
      FORCE)
endif(NOT CMAKE_BUILD_TYPE)

message("Build type:"${CMAKE_BUILD_TYPE})

if (${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    # FIXME: QML refactor, disable test temporary.
    set(BUILD_TESTING OFF)
    set(BUILD_DOCS Off)
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Ofast")
    set(BUILD_DOCS ON CACHE BOOL "Generate doxygen-based documentation")
    # generate qm
    execute_process(COMMAND bash "translate_generation.sh"
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/src)
endif()

# sub directories
add_subdirectory(src/apps)
add_subdirectory(src/dfm-base)
add_subdirectory(src/dfm-extension)
add_subdirectory(src/dfm-framework)
add_subdirectory(src/dfm-gui)
add_subdirectory(src/plugins)
# add_subdirectory(src/external)
# add_subdirectory(src/tools)
add_subdirectory(src/dfm-declarative)

# docs
if (BUILD_DOCS)
    find_package(Doxygen)
    if (DOXYGEN_FOUND)
        add_subdirectory(docs)
    endif()
endif ()

# test
message(STATUS "Enable testing: ${BUILD_TESTING}")
if(BUILD_TESTING)
    enable_testing()
    add_subdirectory(tests)
endif()
