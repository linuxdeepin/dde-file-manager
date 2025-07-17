# dependencies.cmake - Dependencies configuration for dfmplugin-computer
# This file defines the specific dependencies and configuration for the computer plugin

cmake_minimum_required(VERSION 3.10)

# Include DFM plugin configuration module
include(DFMPluginConfig)

# Function to setup computer plugin dependencies
function(dfm_setup_computer_dependencies target_name)
    message(STATUS "DFM: Setting up computer plugin dependencies for: ${target_name}")
    
    # Find required packages
    find_package(PkgConfig REQUIRED)
    find_package(Qt6 REQUIRED COMPONENTS Core)
    pkg_check_modules(Secret REQUIRED libsecret-1)
    
    # Apply default plugin configuration first
    dfm_apply_default_plugin_config(${target_name})
    
    # Add computer-specific dependencies
    target_link_libraries(${target_name} PRIVATE
        Qt6::Core
        ${Secret_LIBRARIES}
    )
    
    # Add include directories
    target_include_directories(${target_name} PRIVATE
        ${Secret_INCLUDE_DIRS}
    )
    
    message(STATUS "DFM: Computer plugin dependencies configured successfully")
endfunction()

message(STATUS "DFM: Computer plugin dependencies configuration loaded")