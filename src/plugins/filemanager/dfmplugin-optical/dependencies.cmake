# dependencies.cmake - Dependencies configuration for dfmplugin-optical
# This file defines the specific dependencies and configuration for the optical plugin

cmake_minimum_required(VERSION 3.10)

# Include DFM plugin configuration module
include(DFMPluginConfig)

# Function to setup optical plugin dependencies
function(dfm_setup_optical_dependencies target_name)
    message(STATUS "DFM: Setting up optical plugin dependencies for: ${target_name}")
    
    # Find required packages
    find_package(Qt6 REQUIRED COMPONENTS Core DBus Widgets Concurrent SvgWidgets)
    
    # Apply default plugin configuration first
    dfm_apply_default_plugin_config(${target_name})
    
    # Add optical-specific dependencies
    target_link_libraries(${target_name} PRIVATE
        Qt6::Core
        Qt6::Widgets
        Qt6::DBus
        Qt6::Concurrent
        Qt6::SvgWidgets
    )
    
    message(STATUS "DFM: Optical plugin dependencies configured successfully")
endfunction()

message(STATUS "DFM: Optical plugin dependencies configuration loaded") 