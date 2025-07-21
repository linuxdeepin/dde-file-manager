# dependencies.cmake - Dependencies configuration for dfmplugin-utils
# This file defines the specific dependencies and configuration for the utils plugin

cmake_minimum_required(VERSION 3.10)

# Include DFM plugin configuration module
include(DFMPluginConfig)

# Function to setup utils plugin dependencies
function(dfm_setup_utils_dependencies target_name)
    message(STATUS "DFM: Setting up utils plugin dependencies for: ${target_name}")
    
    # Find required packages
    find_package(Qt6 REQUIRED COMPONENTS Core SvgWidgets)
    
    # Apply default plugin configuration first
    dfm_apply_default_plugin_config(${target_name})
    
    # Add utils-specific dependencies
    target_link_libraries(${target_name} PRIVATE
        Qt6::SvgWidgets
        DFM::extension
    )
    
    # Add utils-specific definitions
    set(EXTENSIONS_LIB_DIR ${DFM_PLUGIN_DIR}/extensions)
    target_compile_definitions(${target_name} PRIVATE
        EXTENSIONS_PATH="${EXTENSIONS_LIB_DIR}"
    )
    
    # Configure config.h if needed
    if(EXISTS "${DFM_APP_SOURCE_DIR}/config.h.in")
        configure_file(
            "${DFM_APP_SOURCE_DIR}/config.h.in"
            "${CMAKE_CURRENT_BINARY_DIR}/config.h"
        )
        target_include_directories(${target_name} PRIVATE
            ${CMAKE_CURRENT_BINARY_DIR}
        )
    endif()
    
    message(STATUS "DFM: Utils plugin dependencies configured successfully")
endfunction()

message(STATUS "DFM: Utils plugin dependencies configuration loaded") 
