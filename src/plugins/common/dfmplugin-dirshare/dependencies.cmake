# dependencies.cmake - Dependencies configuration for dfmplugin-dirshare
# This file defines the specific dependencies and configuration for the dirshare plugin

cmake_minimum_required(VERSION 3.10)

# Function to setup dirshare plugin dependencies
function(dfm_setup_dirshare_dependencies target_name)
    message(STATUS "DFM: Setting up dirshare plugin dependencies for: ${target_name}")
    
    # Find required packages
    find_package(Qt6 REQUIRED COMPONENTS Core Network)
    
    # Apply default plugin configuration first
    dfm_apply_default_plugin_config(${target_name})
    
    # Add dirshare-specific dependencies
    target_link_libraries(${target_name} PRIVATE
        Qt6::Network
    )
    
    message(STATUS "DFM: Dirshare plugin dependencies configured successfully")
endfunction()

message(STATUS "DFM: Dirshare plugin dependencies configuration loaded") 
