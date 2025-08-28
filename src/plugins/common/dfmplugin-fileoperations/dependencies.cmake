# dependencies.cmake - Dependencies configuration for dfmplugin-fileoperations
# This file defines the specific dependencies and configuration for the fileoperations plugin

cmake_minimum_required(VERSION 3.10)

# Include DFM plugin configuration module
include(DFMPluginConfig)

# Function to setup fileoperations plugin dependencies
function(dfm_setup_fileoperations_dependencies target_name)
    message(STATUS "DFM: Setting up fileoperations plugin dependencies for: ${target_name}")
    
    # Find required packages
    find_package(Qt6 REQUIRED COMPONENTS Core DBus)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(zlib REQUIRED zlib IMPORTED_TARGET)
    
    # Apply default plugin configuration first
    dfm_apply_default_plugin_config(${target_name})
    
    # Add fileoperations-specific dependencies
    target_link_libraries(${target_name} PRIVATE
        Qt6::Core
        Qt6::DBus
        PkgConfig::zlib
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
    
    # Setup fileoperations-specific DBus interfaces
    dfm_setup_fileoperations_dbus_interfaces(${target_name})
    
    message(STATUS "DFM: Fileoperations plugin dependencies configured successfully")
endfunction()

# Function to setup fileoperations-specific DBus interfaces
function(dfm_setup_fileoperations_dbus_interfaces target_name)
    message(STATUS "DFM: Setting up fileoperations DBus interfaces for: ${target_name}")
    
    # Define the DBus interface file paths using DFM_ASSETS_DIR for proper standalone/integrated mode support
    set(OPERATIONS_STACK_MANAGER_INTERFACE_FILE "${DFM_ASSETS_DIR}/dbus/org.deepin.Filemanager.Daemon.OperationsStackManager.xml")
    set(SYNC_INTERFACE_FILE "${DFM_ASSETS_DIR}/dbus/org.deepin.Filemanager.Daemon.Sync.xml")
    
    # Setup OperationsStackManager DBus interface
    if(EXISTS "${OPERATIONS_STACK_MANAGER_INTERFACE_FILE}")
        message(STATUS "DFM: Found OperationsStackManager DBus interface file: ${OPERATIONS_STACK_MANAGER_INTERFACE_FILE}")
        
        # Generate DBus interface
        qt6_add_dbus_interface(OPERATIONS_STACK_MANAGER_INTERFACE_SOURCES ${OPERATIONS_STACK_MANAGER_INTERFACE_FILE} operationsstackmanager_interface_qt6)
        
        # Add generated sources to target
        if(OPERATIONS_STACK_MANAGER_INTERFACE_SOURCES)
            target_sources(${target_name} PRIVATE ${OPERATIONS_STACK_MANAGER_INTERFACE_SOURCES})
            message(STATUS "DFM: Added OperationsStackManager DBus interface sources to target")
        endif()
    else()
        message(WARNING "DFM: OperationsStackManager DBus interface file not found: ${OPERATIONS_STACK_MANAGER_INTERFACE_FILE}")
    endif()
    
    # Setup Sync DBus interface
    if(EXISTS "${SYNC_INTERFACE_FILE}")
        message(STATUS "DFM: Found Sync DBus interface file: ${SYNC_INTERFACE_FILE}")
        
        # Generate DBus interface
        qt6_add_dbus_interface(SYNC_INTERFACE_SOURCES ${SYNC_INTERFACE_FILE} sync_interface_qt6)
        
        # Add generated sources to target
        if(SYNC_INTERFACE_SOURCES)
            target_sources(${target_name} PRIVATE ${SYNC_INTERFACE_SOURCES})
            message(STATUS "DFM: Added Sync DBus interface sources to target")
        endif()
    else()
        message(WARNING "DFM: Sync DBus interface file not found: ${SYNC_INTERFACE_FILE}")
    endif()
endfunction()

message(STATUS "DFM: Fileoperations plugin dependencies configuration loaded")
