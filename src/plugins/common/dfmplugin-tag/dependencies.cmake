# dependencies.cmake - Dependencies configuration for dfmplugin-tag
# This file defines the specific dependencies and configuration for the tag plugin

cmake_minimum_required(VERSION 3.10)

# Include DFM plugin configuration module
include(DFMPluginConfig)

# Function to setup tag plugin dependencies
function(dfm_setup_tag_dependencies target_name)
    message(STATUS "DFM: Setting up tag plugin dependencies for: ${target_name}")
    
    # Find required packages
    find_package(Qt6 REQUIRED COMPONENTS Core DBus)
    
    # Apply default plugin configuration first
    dfm_apply_default_plugin_config(${target_name})
    
    # Add tag-specific dependencies
    target_link_libraries(${target_name} PRIVATE
        Qt6::DBus
    )
    
    # Setup tag-specific DBus interfaces
    dfm_setup_tag_dbus_interfaces(${target_name})
    
    message(STATUS "DFM: Tag plugin dependencies configured successfully")
endfunction()

# Function to setup tag-specific DBus interfaces
function(dfm_setup_tag_dbus_interfaces target_name)
    message(STATUS "DFM: Setting up tag DBus interfaces for: ${target_name}")
    
    # Define the DBus interface file path using DFM_ASSETS_DIR for proper standalone/integrated mode support
    set(DBUS_INTERFACE_FILE "${DFM_ASSETS_DIR}/dbus/org.deepin.Filemanager.Daemon.TagManager.xml")
    
    # Check if the DBus interface file exists
    if(EXISTS "${DBUS_INTERFACE_FILE}")
        message(STATUS "DFM: Found TagManager DBus interface file: ${DBUS_INTERFACE_FILE}")
        
        # Generate DBus interface
        qt6_add_dbus_interface(INTERFACE_SOURCES ${DBUS_INTERFACE_FILE} tagmanager_interface_qt6)
        
        # Add generated sources to target
        if(INTERFACE_SOURCES)
            target_sources(${target_name} PRIVATE ${INTERFACE_SOURCES})
            message(STATUS "DFM: Added TagManager DBus interface sources to target")
        endif()
    else()
        message(WARNING "DFM: TagManager DBus interface file not found: ${DBUS_INTERFACE_FILE}")
    endif()
endfunction()

message(STATUS "DFM: Tag plugin dependencies configuration loaded") 