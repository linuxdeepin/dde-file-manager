cmake_minimum_required(VERSION 3.10)

project(filemanager)
set(CMAKE_CXX_STANDARD 17)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall -Wl,--as-needed")

# options
option(OPT_DISABLE_QDEBUG "Disable Debug Macro" OFF)
option(OPT_ENABLE_BUILD_DOCS "Build develop documents" OFF)
option(OPT_ENABLE_BUILD_UT "Buld unit tests" ON)
option(OPT_ENABLE_QML "Use QML" OFF)

# 安全加固
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fstack-protector-all -Werror=return-type")

# if no debug, can't out in code define key '__FUNCTION__' and so on
add_definitions(-DQT_MESSAGELOGCONTEXT)
# TODO: control it by CI. (Automation Testing)
add_definitions(-DENABLE_TESTING)

# QML or QtWidget
if(OPT_ENABLE_QML)
    message(STATUS "Use QML")
    add_definitions(-DDFM_UI_TYPE_QML)
    # TODO: remove it
    add_definitions(-DCOMPILE_ON_V23)
    set(COMPLIE_ON_V23 TRUE)
    message("COMPILE ON v23")
    find_package(QT NAMES Qt6 REQUIRED COMPONENTS Core Widgets)
    message("Using Qt version: ${QT_VERSION_MAJOR}")
else()
    message(STATUS "Use QtWidget")
    add_definitions(-DDFM_UI_TYPE_QWIDGET)
    find_package(QT NAMES Qt5 REQUIRED COMPONENTS Core Widgets)
    message("Using Qt version: ${QT_VERSION_MAJOR}")
    #Indentify the version
    find_package(DtkCMake REQUIRED)
    if (${DEEPIN_OS_VERSION} MATCHES "23")
        add_definitions(-DCOMPILE_ON_V23)
        set(COMPLIE_ON_V23 TRUE)
        message("COMPILE ON v23")
    else()
        add_definitions(-DCOMPILE_ON_V20)
        set(COMPLIE_ON_V20 TRUE)
        message("COMPILE ON v20")
    endif()
endif()

# Select major Qt version.
if(QT_VERSION_MAJOR EQUAL "6")
    set(DFM_BUILD_WITH_QT6 ON)
    set(DTK_VERSION_MAJOR 6)
else()
    set(DTK_VERSION_MAJOR "")
endif()

# check arch
if (${CMAKE_SYSTEM_PROCESSOR} MATCHES "mips")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -z noexecstack -z relro")
    add_definitions(-DARCH_MIPSEL)
endif()

# CMake modules
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(install_plugin_quick_module)
include(install_dconfig)
include(DFMInstallDirs)
include(GNUInstallDirs)

# build type
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Release" CACHE STRING
      "Choose the type of build, options are: Debug Release RelWithDebInfo MinSizeRel."
      FORCE)
endif(NOT CMAKE_BUILD_TYPE)
message("Build type:"${CMAKE_BUILD_TYPE})

if (${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    set(BUILD_TESTING ON)
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Ofast")
    # generate qm
    execute_process(COMMAND bash "translate_generation.sh"
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/src)
endif()

# sub directories
add_subdirectory(src/apps)
add_subdirectory(src/dfm-base)
add_subdirectory(src/dfm-extension)
add_subdirectory(src/dfm-framework)
add_subdirectory(src/plugins)
if(OPT_ENABLE_QML)
    add_subdirectory(src/dfm-gui)
    add_subdirectory(src/dfm-declarative)
else()
    # TODO: adapt to it
    add_subdirectory(src/external)
    add_subdirectory(src/tools)
endif()

# docs
if(OPT_ENABLE_BUILD_DOCS AND NOT OPT_ENABLE_QML)
    find_package(Doxygen)
    if (DOXYGEN_FOUND)
        add_subdirectory(docs)
    endif()
endif()

# test
if(OPT_ENABLE_BUILD_UT AND NOT OPT_ENABLE_QML)
    message(STATUS "Enable testing: ${BUILD_TESTING}")
    if(BUILD_TESTING)
        enable_testing()
        add_subdirectory(tests)
    endif()
endif()
