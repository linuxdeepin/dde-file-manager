# dependencies.cmake - Dependencies configuration for dfmplugin-smbbrowser
# This file defines the specific dependencies and configuration for the smbbrowser plugin

cmake_minimum_required(VERSION 3.10)

# Include DFM plugin configuration module
include(DFMPluginConfig)

# Function to setup smbbrowser plugin dependencies
function(dfm_setup_smbbrowser_dependencies target_name)
    message(STATUS "DFM: Setting up smbbrowser plugin dependencies for: ${target_name}")
    
    # Find required packages
    find_package(Qt6 REQUIRED COMPONENTS Network)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(LibSecret REQUIRED libsecret-1 IMPORTED_TARGET)
    
    # Apply default plugin configuration first
    dfm_apply_default_plugin_config(${target_name})
    
    # Add smbbrowser-specific dependencies
    target_link_libraries(${target_name} PRIVATE
        Qt6::Network
        PkgConfig::LibSecret
    )
    
    message(STATUS "DFM: Smbbrowser plugin dependencies configured successfully")
endfunction()

message(STATUS "DFM: Smbbrowser plugin dependencies configuration loaded")