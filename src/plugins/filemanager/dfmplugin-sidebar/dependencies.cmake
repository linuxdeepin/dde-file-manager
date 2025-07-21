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

    message(STATUS "DFM: Sidebar is implemented with tree view")
    message(STATUS "DFM: Sidebar plugin dependencies configured successfully")
endfunction()


message(STATUS "DFM: Sidebar plugin dependencies configuration loaded") 
