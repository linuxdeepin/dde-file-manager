# dependencies.cmake - Dependencies configuration for dfmplugin-trash
# This file defines the specific dependencies and configuration for the trash plugin

cmake_minimum_required(VERSION 3.10)

# Include DFM plugin configuration module
include(DFMPluginConfig)

# Function to setup trash plugin dependencies
function(dfm_setup_trash_dependencies target_name)
    message(STATUS "DFM: Setting up trash plugin dependencies for: ${target_name}")
    
    # Find required packages
    find_package(Qt6 REQUIRED COMPONENTS Xml)
    
    # Apply default plugin configuration first
    dfm_apply_default_plugin_config(${target_name})
    
    # Add trash-specific dependencies
    target_link_libraries(${target_name} PRIVATE
        Qt6::Xml
    )
    
    message(STATUS "DFM: Trash plugin dependencies configured successfully")
endfunction()

message(STATUS "DFM: Trash plugin dependencies configuration loaded")