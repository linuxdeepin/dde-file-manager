# DFMCommon.cmake - Common build configuration for DDE File Manager
# This module provides shared functionality for both main project and autotests

cmake_minimum_required(VERSION 3.10)

# Standard C++ settings
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Qt settings
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Common compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall -Wl,--as-needed")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fstack-protector-all -Werror=return-type")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wl,-z,relro -Wl,-z,now")

# Common definitions
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(COMPILE_ON_V2X TRUE)
add_definitions(-DQT_MESSAGELOGCONTEXT)
add_definitions(-DENABLE_TESTING)
add_definitions(-DCOMPILE_ON_V2X)

# Architecture-specific settings
if (${CMAKE_SYSTEM_PROCESSOR} MATCHES "mips")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -z noexecstack -z relro")
    add_definitions(-DARCH_MIPSEL)
endif()

# Build type configuration
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING
        "Choose the type of build, options are: Debug Release RelWithDebInfo MinSizeRel."
        FORCE)
endif()

if (${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    set(BUILD_TESTING ON)
    set(CMAKE_VERBOSE_MAKEFILE ON)
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Ofast")
endif()

# Function to detect if we're in standalone test mode
function(dfm_detect_standalone_mode)
    if(DEFINED DFM_STANDALONE_TEST)
        return()
    endif()
    
    # Check if we're building from autotests directory
    get_filename_component(CURRENT_DIR_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
    if(CURRENT_DIR_NAME STREQUAL "autotests")
        set(DFM_STANDALONE_TEST ON CACHE BOOL "Building tests in standalone mode")
        message(STATUS "DFM: Detected standalone test mode")
    else()
        set(DFM_STANDALONE_TEST OFF CACHE BOOL "Building tests in integrated mode")
        message(STATUS "DFM: Detected integrated build mode")
    endif()
endfunction()

# Function to setup project paths
function(dfm_setup_paths)
    dfm_detect_standalone_mode()
    
    if(DFM_STANDALONE_TEST)
        # Standalone mode - autotests is the root
        set(DFM_PROJECT_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/.." CACHE PATH "Project root directory")
        set(DFM_SOURCE_DIR "${DFM_PROJECT_ROOT}/src" CACHE PATH "Source directory")
        set(DFM_INCLUDE_DIR "${DFM_PROJECT_ROOT}/include" CACHE PATH "Include directory")
        set(DFM_ASSETS_DIR "${DFM_PROJECT_ROOT}/assets" CACHE PATH "Assets directory")
        set(DFM_3RDPARTY_DIR "${DFM_PROJECT_ROOT}/3rdparty" CACHE PATH "3rdparty directory")
        set(DFM_APP_SOURCE_DIR "${DFM_PROJECT_ROOT}/src/apps" CACHE PATH "App source directory")
        
        message(STATUS "DFM: Standalone test paths configured")
        message(STATUS "  - Project root: ${DFM_PROJECT_ROOT}")
        message(STATUS "  - Source dir: ${DFM_SOURCE_DIR}")
        message(STATUS "  - Include dir: ${DFM_INCLUDE_DIR}")
        message(STATUS "  - DFM_ASSETS_DIR dir: ${DFM_ASSETS_DIR}")
        message(STATUS "  - DFM_3RDPARTY_DIR dir: ${DFM_3RDPARTY_DIR}")
        message(STATUS "  - DFM_APP_SOURCE_DIR: ${DFM_APP_SOURCE_DIR}")

    else()
        # Integrated mode - main project is the root
        set(DFM_PROJECT_ROOT "${CMAKE_SOURCE_DIR}" CACHE PATH "Project root directory")
        set(DFM_SOURCE_DIR "${CMAKE_SOURCE_DIR}/src" CACHE PATH "Source directory")
        set(DFM_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/include" CACHE PATH "Include directory")
        set(DFM_ASSETS_DIR "${CMAKE_SOURCE_DIR}/assets" CACHE PATH "Assets directory")
        set(DFM_3RDPARTY_DIR "${CMAKE_SOURCE_DIR}/3rdparty" CACHE PATH "3rdparty directory")
        set(DFM_APP_SOURCE_DIR "${CMAKE_SOURCE_DIR}/src/apps" CACHE PATH "App source directory")
        
        message(STATUS "DFM: Integrated build paths configured")
    endif()
    
    # Make variables available to parent scope
    set(DFM_PROJECT_ROOT ${DFM_PROJECT_ROOT} PARENT_SCOPE)
    set(DFM_SOURCE_DIR ${DFM_SOURCE_DIR} PARENT_SCOPE)
    set(DFM_INCLUDE_DIR ${DFM_INCLUDE_DIR} PARENT_SCOPE)
    set(DFM_ASSETS_DIR ${DFM_ASSETS_DIR} PARENT_SCOPE)
    set(DFM_3RDPARTY_DIR ${DFM_3RDPARTY_DIR} PARENT_SCOPE)
    set(DFM_APP_SOURCE_DIR ${DFM_APP_SOURCE_DIR} PARENT_SCOPE)
    set(DFM_STANDALONE_TEST ${DFM_STANDALONE_TEST} PARENT_SCOPE)
endfunction()

# Function to find DFM libraries
function(dfm_find_libraries)
    if(DFM_STANDALONE_TEST)
        # In standalone mode, try to find installed libraries first
        find_package(PkgConfig QUIET)
        
        # Try to find dfm6-base
        if(PkgConfig_FOUND)
            pkg_check_modules(DFM6_BASE QUIET dfm6-base)
            pkg_check_modules(DFM6_FRAMEWORK QUIET dfm6-framework)
            pkg_check_modules(DFM_EXTENSION QUIET dfm-extension)
        endif()
        
        # Fallback to cmake config files
        if(NOT DFM6_BASE_FOUND)
            find_package(dfm6-base QUIET)
        endif()
        if(NOT DFM6_FRAMEWORK_FOUND)
            find_package(dfm6-framework QUIET)
        endif()
        if(NOT DFM_EXTENSION_FOUND)
            find_package(dfm-extension QUIET)
        endif()
        
        # If still not found, try to use build tree
        if(NOT DFM6_BASE_FOUND AND NOT dfm6-base_FOUND)
            message(STATUS "DFM: Looking for libraries in build tree...")
            # Add the build directory to CMAKE_PREFIX_PATH
            list(APPEND CMAKE_PREFIX_PATH "${DFM_PROJECT_ROOT}/build")
            list(APPEND CMAKE_PREFIX_PATH "${DFM_PROJECT_ROOT}/obj-x86_64-linux-gnu")
            set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} PARENT_SCOPE)
        endif()
        
        message(STATUS "DFM: Library search completed for standalone mode")
    else()
        # In integrated mode, libraries are built as part of the same project
        message(STATUS "DFM: Using integrated build targets")
    endif()
endfunction()

# Function to setup common Qt and system dependencies
function(dfm_setup_common_dependencies)
    # Qt6 dependencies
    find_package(Qt6 COMPONENTS Core Widgets Gui Concurrent DBus Sql Network Test REQUIRED)
    
    # DTK dependencies
    find_package(Dtk6 COMPONENTS Core Widget Gui REQUIRED)
    
    # System dependencies
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(gio REQUIRED gio-unix-2.0 IMPORTED_TARGET)
    pkg_check_modules(mount REQUIRED mount IMPORTED_TARGET)
    pkg_check_modules(LIBHEIF REQUIRED libheif)
    
    # X11 dependency - use pkg_search_module like the main project
    pkg_search_module(X11 REQUIRED x11 IMPORTED_TARGET)
    
    # DFM IO dependencies
    find_package(dfm6-io REQUIRED)
    find_package(dfm6-mount REQUIRED)
    find_package(dfm6-burn REQUIRED)
    
    message(STATUS "DFM: Common dependencies configured")
endfunction()

# Function to setup DFM-specific paths and definitions
function(dfm_setup_definitions)
    # DBUS XML directory
    set(DFM_DBUS_XML_DIR "${DFM_ASSETS_DIR}/dbus" CACHE PATH "DBUS XML directory")
    set(DFM_DBUS_XML_DIR ${DFM_DBUS_XML_DIR} PARENT_SCOPE)
    
    # Common definitions
    add_definitions(-DAPPSHAREDIR="${CMAKE_INSTALL_PREFIX}/share/dde-file-manager")
    
    message(STATUS "DFM: Definitions configured")
    message(STATUS "  - DBUS XML dir: ${DFM_DBUS_XML_DIR}")
endfunction()

# Main function to initialize DFM common environment
function(dfm_init_common)
    message(STATUS "DFM: Initializing common environment...")
    
    dfm_setup_paths()
    dfm_find_libraries()
    dfm_setup_common_dependencies()
    dfm_setup_definitions()
    
    message(STATUS "DFM: Common environment initialized successfully")
endfunction()

# Convenience function for tests to get test utilities path
function(dfm_get_test_utils_path result_var)
    set(${result_var} "${DFM_3RDPARTY_DIR}/testutils" PARENT_SCOPE)
endfunction()

# Function to create library targets that work in both modes
function(dfm_create_library_target target_name)
    if(DFM_STANDALONE_TEST)
        # In standalone mode, create imported targets if not already available
        if(NOT TARGET ${target_name})
            add_library(${target_name} SHARED IMPORTED)
            # Try to find the library in standard locations
            find_library(${target_name}_LIBRARY NAMES ${target_name} lib${target_name})
            if(${target_name}_LIBRARY)
                set_target_properties(${target_name} PROPERTIES
                    IMPORTED_LOCATION ${${target_name}_LIBRARY})
                message(STATUS "DFM: Created imported target ${target_name}")
            else()
                message(WARNING "DFM: Could not find library ${target_name}")
            endif()
        endif()
    endif()
    # In integrated mode, targets are created by the main build system
endfunction()

# Include library configuration module
include(DFMLibraryConfig)

message(STATUS "DFM: Common CMake module loaded") 
