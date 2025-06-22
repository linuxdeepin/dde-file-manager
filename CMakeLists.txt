cmake_minimum_required(VERSION 3.10)

project(filemanager)
set(CMAKE_CXX_STANDARD 17)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_FLAGS "")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall -Wl,--as-needed")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fstack-protector-all -Werror=return-type")

# options
option(OPT_DISABLE_QDEBUG "Disable Debug Macro" OFF)
option(OPT_ENABLE_BUILD_DOCS "Build develop documents" OFF)
option(OPT_ENABLE_BUILD_UT "Buld unit tests" ON)

# if no debug, can't out in code define key '__FUNCTION__' and so on
add_definitions(-DQT_MESSAGELOGCONTEXT)
# TODO: control it by CI. (Automation Testing)
add_definitions(-DENABLE_TESTING)

set(COMPILE_ON_V2X TRUE)
add_definitions(-DCOMPILE_ON_V2X)    # TODO: remove it
message("COMPILE ON v2X")

# check arch
if (${CMAKE_SYSTEM_PROCESSOR} MATCHES "mips")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -z noexecstack -z relro")
    add_definitions(-DARCH_MIPSEL)
endif()

# CMake modules
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

include(install_dconfig)
include(install_dbus_service)
include(GNUInstallDirs)
include(DFMInstallDirs)

# build type
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Release" CACHE STRING
      "Choose the type of build, options are: Debug Release RelWithDebInfo MinSizeRel."
      FORCE)
endif(NOT CMAKE_BUILD_TYPE)
message("Build type:" ${CMAKE_BUILD_TYPE})

if (${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    set(BUILD_TESTING ON)
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Ofast")
endif()

message(STATUS "CMAKE_CXX_FLAGS: ${CMAKE_CXX_FLAGS}")

# sub directories
add_subdirectory(src/apps)
add_subdirectory(src/dfm-base)
add_subdirectory(src/dfm-extension)
add_subdirectory(src/dfm-framework)
add_subdirectory(src/plugins)
add_subdirectory(src/external)
add_subdirectory(src/tools)
add_subdirectory(src/services)

# docs
if(OPT_ENABLE_BUILD_DOCS)
    find_package(Doxygen)
    if (DOXYGEN_FOUND)
        add_subdirectory(docs)
    endif()
endif()

# test
if(OPT_ENABLE_BUILD_UT)
    message(STATUS "Enable testing: ${BUILD_TESTING}")
    if(BUILD_TESTING)
        enable_testing()
        add_subdirectory(tests2)
    endif()
endif()

