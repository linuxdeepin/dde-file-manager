# dependencies.cmake - Dependencies configuration for diskencrypt service
# This file defines the specific dependencies and configuration for the disk encryption service

cmake_minimum_required(VERSION 3.10)

# Function to setup diskencrypt service dependencies
function(dfm_setup_diskencrypt_dependencies target_name)
    message(STATUS "DFM: Setting up diskencrypt service dependencies for: ${target_name}")
    
    # Find required packages
    find_package(PkgConfig REQUIRED)
    find_package(Qt6 REQUIRED COMPONENTS Core Concurrent DBus)
    find_package(deepin-qdbus-service REQUIRED)
    
    # Find system dependencies using pkg-config
    pkg_check_modules(CryptSetup REQUIRED libcryptsetup)
    pkg_check_modules(DevMapper REQUIRED devmapper)
    pkg_check_modules(PolkitAgent REQUIRED polkit-agent-1 IMPORTED_TARGET)
    pkg_check_modules(PolkitQt6 REQUIRED polkit-qt6-1 IMPORTED_TARGET)
    
    # Apply default service configuration first (this provides DFM6::base, Qt6::Core, Qt6::DBus)
    dfm_apply_default_service_config(${target_name})
    
    # Add diskencrypt-specific dependencies
    target_link_libraries(${target_name} PRIVATE
        Qt6::Concurrent
        PkgConfig::PolkitAgent
        PkgConfig::PolkitQt6
        deepin-qdbus-service
        ${CryptSetup_LIBRARIES}
        ${DevMapper_LIBRARIES}
    )
    
    # Add diskencrypt-specific include directories
    target_include_directories(${target_name} PRIVATE
        ${PROJECT_SOURCE_DIR}
        ${CryptSetup_INCLUDE_DIRS}
    )
    
    # Add compile definitions
    target_compile_definitions(${target_name} PRIVATE
        SERVICE_CONFIG_DIR="${CMAKE_INSTALL_PREFIX}/share/deepin-service-manager/"
    )
    
    # Setup DBus interface generation
    dfm_setup_diskencrypt_dbus_interfaces(${target_name})
    
    message(STATUS "DFM: Disk encryption service dependencies configured successfully")
endfunction()

# Function to setup diskencrypt-specific DBus interfaces
function(dfm_setup_diskencrypt_dbus_interfaces target_name)
    message(STATUS "DFM: Setting up diskencrypt DBus interfaces for: ${target_name}")
    
    # Define the DBus interface file path using DFM_ASSETS_DIR
    set(DBUS_INTERFACE_FILE "${DFM_ASSETS_DIR}/dbus/org.deepin.Filemanager.DiskEncrypt.xml")
    
    # Check if the DBus interface file exists
    if(EXISTS "${DBUS_INTERFACE_FILE}")
        message(STATUS "DFM: Found DiskEncrypt DBus interface file: ${DBUS_INTERFACE_FILE}")
        
        # Generate DBus interface and adaptor
        # qt6_generate_dbus_interface(
        #     dbus/diskencryptsetup.h
        #     ${DBUS_INTERFACE_FILE}
        #     OPTIONS -M -S
        # )
        
        qt6_add_dbus_adaptor(ADAPTOR_SOURCES
            ${DBUS_INTERFACE_FILE}
            dbus/diskencryptsetup.h
            DiskEncryptSetup)
        
        # Add generated sources to target
        if(ADAPTOR_SOURCES)
            target_sources(${target_name} PRIVATE ${ADAPTOR_SOURCES})
            message(STATUS "DFM: Added DiskEncrypt DBus adaptor sources to target")
        endif()
    else()
        message(WARNING "DFM: DiskEncrypt DBus interface file not found: ${DBUS_INTERFACE_FILE}")
    endif()
endfunction()

message(STATUS "DFM: Disk encryption service dependencies configuration loaded") 
