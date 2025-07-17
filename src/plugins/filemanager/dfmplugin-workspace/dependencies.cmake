# dependencies.cmake - Dependencies configuration for dfmplugin-workspace
# This file defines the specific dependencies and configuration for the workspace plugin

cmake_minimum_required(VERSION 3.10)

# Include DFM plugin configuration module
include(DFMPluginConfig)

# Function to setup workspace plugin dependencies
function(dfm_setup_workspace_dependencies target_name)
    message(STATUS "DFM: Setting up workspace plugin dependencies for: ${target_name}")
    
    # Find required packages
    find_package(Dtk6 COMPONENTS Widget REQUIRED)

    # Apply default plugin configuration first
    dfm_apply_default_plugin_config(${target_name})

    # Add workspace-specific include directories for Qt private headers
    target_include_directories(${target_name} PRIVATE
        ${Qt6Widgets_PRIVATE_INCLUDE_DIRS}
    )

    message(STATUS "DFM: Workspace plugin dependencies configured successfully")
endfunction()

message(STATUS "DFM: Workspace plugin dependencies configuration loaded")
