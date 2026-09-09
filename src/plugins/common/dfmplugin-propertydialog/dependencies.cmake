# dependencies.cmake - Dependencies configuration for dfmplugin-propertydialog
# This file defines the specific dependencies and configuration for the propertydialog plugin

cmake_minimum_required(VERSION 3.10)

# Include DFM plugin configuration module
include(DFMPluginConfig)

# Function to setup propertydialog plugin dependencies
function(dfm_setup_propertydialog_dependencies target_name)
    message(STATUS "DFM: Setting up propertydialog plugin dependencies for: ${target_name}")

    # Find required packages
    find_package(Qt6 REQUIRED COMPONENTS Core Svg)

    # Apply default plugin configuration first
    dfm_apply_default_plugin_config(${target_name})

    # Add propertydialog-specific dependencies
    target_link_libraries(${target_name} PRIVATE
        Qt6::Svg
    )

    message(STATUS "DFM: Propertydialog plugin dependencies configured successfully")
endfunction()

message(STATUS "DFM: Propertydialog plugin dependencies configuration loaded")
