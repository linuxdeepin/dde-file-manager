# dependencies.cmake - Dependencies configuration for accesscontrol service
# This file defines the specific dependencies and configuration for the access control service

cmake_minimum_required(VERSION 3.10)

# Function to setup accesscontrol service dependencies
function(dfm_setup_accesscontrol_dependencies target_name)
    message(STATUS "DFM: Setting up accesscontrol service dependencies for: ${target_name}")

    # Find required packages
    find_package(PkgConfig REQUIRED)
    find_package(Qt6 REQUIRED COMPONENTS DBus)

    # Find system dependencies using pkg-config
    pkg_search_module(crypt REQUIRED libcryptsetup IMPORTED_TARGET)
    pkg_check_modules(PolkitAgent REQUIRED polkit-agent-1 IMPORTED_TARGET)
    pkg_check_modules(PolkitQt6 REQUIRED polkit-qt6-1 IMPORTED_TARGET)

    # Apply default service configuration first
    dfm_apply_default_service_config(${target_name})

    # Include service common utilities and apply shared polkit helper
    include(${DFM_SOURCE_DIR}/services/DFMServiceCommon.cmake)
    dfm_apply_service_polkit_to_target(${target_name})

    # Add accesscontrol-specific dependencies
    target_link_libraries(${target_name} PRIVATE
        Qt6::DBus
        PkgConfig::crypt
        PkgConfig::PolkitAgent
        PkgConfig::PolkitQt6
    )

    # Setup DBus adaptor for accesscontrol
    dfm_setup_accesscontrol_dbus_interfaces(${target_name})

    message(STATUS "DFM: Access control service dependencies configured successfully")
endfunction()

# Function to setup accesscontrol-specific DBus interfaces
function(dfm_setup_accesscontrol_dbus_interfaces target_name)
    message(STATUS "DFM: Setting up accesscontrol DBus interfaces for: ${target_name}")
    
    # Define the DBus interface file path using DFM_ASSETS_DIR
    set(DBUS_INTERFACE_FILE "${DFM_ASSETS_DIR}/dbus/org.deepin.Filemanager.AccessControl.xml")
    
    # Check if the DBus interface file exists
    if(EXISTS "${DBUS_INTERFACE_FILE}")
        message(STATUS "DFM: Found AccessControl DBus interface file: ${DBUS_INTERFACE_FILE}")
        
        # Generate DBus adaptor
        qt6_add_dbus_adaptor(ADAPTOR_SOURCES ${DBUS_INTERFACE_FILE}
            accesscontroldbus.h AccessControlDBus)
        
        # Add generated sources to target
        if(ADAPTOR_SOURCES)
            target_sources(${target_name} PRIVATE ${ADAPTOR_SOURCES})
            message(STATUS "DFM: Added AccessControl DBus adaptor sources to target")
        endif()
    else()
        message(WARNING "DFM: AccessControl DBus interface file not found: ${DBUS_INTERFACE_FILE}")
    endif()
endfunction()

message(STATUS "DFM: Access control service dependencies configuration loaded") 
