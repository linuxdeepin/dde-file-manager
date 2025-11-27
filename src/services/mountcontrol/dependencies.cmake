# dependencies.cmake - Dependencies configuration for mountcontrol service
# This file defines the specific dependencies and configuration for the mount control service

cmake_minimum_required(VERSION 3.10)

# Function to setup mountcontrol service dependencies
function(dfm_setup_mountcontrol_dependencies target_name)
    message(STATUS "DFM: Setting up mountcontrol service dependencies for: ${target_name}")

    # Find required packages
    find_package(PkgConfig REQUIRED)
    find_package(Qt6 REQUIRED COMPONENTS DBus)

    # Find system dependencies using pkg-config
    pkg_check_modules(PolkitAgent REQUIRED polkit-agent-1)
    pkg_check_modules(PolkitQt6 REQUIRED polkit-qt6-1)
    pkg_check_modules(mount REQUIRED mount IMPORTED_TARGET)

    # Apply default service configuration first
    dfm_apply_default_service_config(${target_name})

    # Include service common utilities and apply shared polkit helper
    include(${DFM_SOURCE_DIR}/services/DFMServiceCommon.cmake)
    dfm_apply_service_polkit_to_target(${target_name})

    # Add mountcontrol-specific dependencies
    target_link_libraries(${target_name} PRIVATE
        Qt6::DBus
        ${PolkitAgent_LIBRARIES}
        ${PolkitQt6_LIBRARIES}
        PkgConfig::mount
    )

    # Setup DBus adaptor for mountcontrol
    dfm_setup_mountcontrol_dbus_interfaces(${target_name})

    message(STATUS "DFM: Mount control service dependencies configured successfully")
endfunction()

# Function to setup mountcontrol-specific DBus interfaces
function(dfm_setup_mountcontrol_dbus_interfaces target_name)
    message(STATUS "DFM: Setting up mountcontrol DBus interfaces for: ${target_name}")
    
    # Define the DBus interface file path using DFM_ASSETS_DIR
    set(DBUS_INTERFACE_FILE "${DFM_ASSETS_DIR}/dbus/org.deepin.Filemanager.MountControl.xml")
    
    # Check if the DBus interface file exists
    if(EXISTS "${DBUS_INTERFACE_FILE}")
        message(STATUS "DFM: Found MountControl DBus interface file: ${DBUS_INTERFACE_FILE}")
        
        # Generate DBus adaptor
        qt6_add_dbus_adaptor(ADAPTOR_SOURCES ${DBUS_INTERFACE_FILE}
            mountcontroldbus.h MountControlDBus)
        
        # Add generated sources to target
        if(ADAPTOR_SOURCES)
            target_sources(${target_name} PRIVATE ${ADAPTOR_SOURCES})
            message(STATUS "DFM: Added MountControl DBus adaptor sources to target")
        endif()
    else()
        message(WARNING "DFM: MountControl DBus interface file not found: ${DBUS_INTERFACE_FILE}")
    endif()
endfunction()

message(STATUS "DFM: Mount control service dependencies configuration loaded") 