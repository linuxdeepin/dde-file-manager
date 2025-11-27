# dependencies.cmake - Dependencies configuration for tpmcontrol service
# This file defines the specific dependencies and configuration for the TPM control service

cmake_minimum_required(VERSION 3.10)

# Function to setup tpmcontrol service dependencies
function(dfm_setup_tpmcontrol_dependencies target_name)
    message(STATUS "DFM: Setting up tpmcontrol service dependencies for: ${target_name}")

    # Find required packages
    find_package(PkgConfig REQUIRED)
    find_package(Qt6 REQUIRED COMPONENTS DBus)

    # Find system dependencies using pkg-config
    pkg_check_modules(PolkitAgent REQUIRED polkit-agent-1)
    pkg_check_modules(PolkitQt6 REQUIRED polkit-qt6-1)

    # Apply default service configuration first
    dfm_apply_default_service_config(${target_name})

    # Include service common utilities and apply shared polkit helper
    include(${DFM_SOURCE_DIR}/services/DFMServiceCommon.cmake)
    dfm_apply_service_polkit_to_target(${target_name})

    # Add tpmcontrol-specific dependencies
    target_link_libraries(${target_name} PRIVATE
        Qt6::DBus
        ${PolkitAgent_LIBRARIES}
        ${PolkitQt6_LIBRARIES}
    )

    # Setup DBus adaptor for tpmcontrol
    dfm_setup_tpmcontrol_dbus_interfaces(${target_name})

    message(STATUS "DFM: TPM control service dependencies configured successfully")
endfunction()

# Function to setup tpmcontrol-specific DBus interfaces
function(dfm_setup_tpmcontrol_dbus_interfaces target_name)
    message(STATUS "DFM: Setting up tpmcontrol DBus interfaces for: ${target_name}")

    # Define the DBus interface file path using DFM_ASSETS_DIR
    set(DBUS_INTERFACE_FILE "${DFM_ASSETS_DIR}/dbus/org.deepin.Filemanager.TPMControl.xml")

    # Check if the DBus interface file exists
    if(EXISTS "${DBUS_INTERFACE_FILE}")
        message(STATUS "DFM: Found TPMControl DBus interface file: ${DBUS_INTERFACE_FILE}")

        # Generate DBus adaptor
        qt6_add_dbus_adaptor(ADAPTOR_SOURCES ${DBUS_INTERFACE_FILE}
            tpmcontroldbus.h TPMControlDBus)

        # Add generated sources to target
        if(ADAPTOR_SOURCES)
            target_sources(${target_name} PRIVATE ${ADAPTOR_SOURCES})
            message(STATUS "DFM: Added TPMControl DBus adaptor sources to target")
        endif()
    else()
        message(WARNING "DFM: TPMControl DBus interface file not found: ${DBUS_INTERFACE_FILE}")
    endif()
endfunction()

message(STATUS "DFM: TPM control service dependencies configuration loaded")
