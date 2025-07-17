# dependencies.cmake - Dependencies configuration for dfmplugin-sidebar
# This file defines the specific dependencies and configuration for the sidebar plugin

cmake_minimum_required(VERSION 3.10)

# Include DFM plugin configuration module
include(DFMPluginConfig)

# Function to setup sidebar plugin dependencies
function(dfm_setup_sidebar_dependencies target_name)
    message(STATUS "DFM: Setting up sidebar plugin dependencies for: ${target_name}")
    
    # Find required packages
    find_package(Qt6 COMPONENTS Core REQUIRED)
    
    # Apply default plugin configuration first
    dfm_apply_default_plugin_config(${target_name})
    
    # Add sidebar-specific definitions and configurations
    target_compile_definitions(${target_name} PRIVATE
        SIDEBAR_TREEVIEW
        TREEVIEW
    )
    
    # Add sidebar-specific include directories based on configuration
    target_include_directories(${target_name} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/treeviews
        ${CMAKE_CURRENT_SOURCE_DIR}/treemodels
    )
    
    message(STATUS "DFM: Sidebar is implemented with tree view")
    message(STATUS "DFM: Sidebar plugin dependencies configured successfully")
endfunction()

# Function to setup sidebar source files based on configuration
function(dfm_setup_sidebar_sources target_name)
    message(STATUS "DFM: Setting up sidebar source files for: ${target_name}")
    
    # Define sidebar source files for tree view implementation
    FILE(GLOB SIDEBAR_FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/utils/*.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/utils/*.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/events/*.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/events/*.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/treemodels/*.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/treemodels/*.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/treeviews/*.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/treeviews/*.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/treeviews/private/*.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/*.json"
    )
    
    # Add source files to target
    target_sources(${target_name} PRIVATE ${SIDEBAR_FILES})
    
    message(STATUS "DFM: Sidebar source files configured successfully")
endfunction()

message(STATUS "DFM: Sidebar plugin dependencies configuration loaded") 