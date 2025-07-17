# dependencies.cmake - Dependencies configuration for dfmplugin-disk-encrypt-entry
# This file defines the specific dependencies and configuration for the disk-encrypt-entry plugin

cmake_minimum_required(VERSION 3.10)

# Function to setup disk-encrypt-entry plugin dependencies
function(dfm_setup_disk_encrypt_entry_dependencies target_name)
    message(STATUS "DFM: Setting up disk-encrypt-entry plugin dependencies for: ${target_name}")
    
    # Find required packages
    find_package(Qt6 REQUIRED COMPONENTS Core Widgets Concurrent LinguistTools)
    
    # Apply default plugin configuration first
    dfm_apply_default_plugin_config(${target_name})
    
    # Add disk-encrypt-entry-specific dependencies
    target_link_libraries(${target_name} PRIVATE
        Qt6::Core
        Qt6::Widgets
        Qt6::Concurrent
    )
    
    # Add custom definitions
    target_compile_definitions(${target_name} PRIVATE DFMPLUGIN_DISK_ENCRYPT_LIBRARY)
    
    message(STATUS "DFM: Disk-encrypt-entry plugin dependencies configured successfully")
endfunction()

message(STATUS "DFM: Disk-encrypt-entry plugin dependencies configuration loaded")
