# DFMServiceConfig.cmake - Service configuration management for DDE File Manager
# This module provides centralized service dependency configuration

cmake_minimum_required(VERSION 3.10)

# Function to apply default service configuration for simple services
function(dfm_apply_default_service_config target_name)
    message(STATUS "DFM: Applying default service configuration for: ${target_name}")
    
    # Default service dependencies - covers most common service needs
    target_link_libraries(${target_name} PRIVATE
        DFM6::base
        Qt6::Core
        Qt6::DBus
    )
    
    message(STATUS "DFM: Default service configuration applied successfully")
endfunction()

# Function to setup service with enhanced configuration
function(dfm_configure_service target_name service_name service_path)
    message(STATUS "DFM: Configuring service: ${service_name} for target: ${target_name}")
    
    # Check if service has custom dependencies.cmake
    set(DEPENDENCIES_FILE "${service_path}/dependencies.cmake")
    
    if(EXISTS "${DEPENDENCIES_FILE}")
        message(STATUS "DFM: Found custom dependencies file: ${DEPENDENCIES_FILE}")
        include("${DEPENDENCIES_FILE}")
        
        # Call the service-specific configuration function
        set(CONFIG_FUNCTION "dfm_setup_${service_name}_dependencies")
        if(COMMAND ${CONFIG_FUNCTION})
            message(STATUS "DFM: Calling custom configuration function: ${CONFIG_FUNCTION}")
            cmake_language(CALL ${CONFIG_FUNCTION} ${target_name})
        else()
            message(WARNING "DFM: Custom dependencies file found but no configuration function: ${CONFIG_FUNCTION}")
            dfm_apply_default_service_config(${target_name})
        endif()
    else()
        message(STATUS "DFM: No custom dependencies found, using default configuration")
        dfm_apply_default_service_config(${target_name})
    endif()
    
    message(STATUS "DFM: Service configuration completed for: ${service_name}")
endfunction()

# Function to setup service DBus interfaces (common utility)
function(dfm_setup_service_dbus_interface target_name service_name interface_name)
    message(STATUS "DFM: Setting up DBus interface for service: ${service_name}")
    
    # Define the DBus interface file path using DFM_ASSETS_DIR
    set(DBUS_INTERFACE_FILE "${DFM_ASSETS_DIR}/dbus/org.deepin.Filemanager.${interface_name}.xml")
    
    # Check if the DBus interface file exists
    if(EXISTS "${DBUS_INTERFACE_FILE}")
        message(STATUS "DFM: Found ${interface_name} DBus interface file: ${DBUS_INTERFACE_FILE}")
        
        # Generate DBus adaptor
        qt6_add_dbus_adaptor(ADAPTOR_SOURCES ${DBUS_INTERFACE_FILE}
            ${service_name}dbus.h ${service_name}DBus)
        
        # Add generated sources to target
        if(ADAPTOR_SOURCES)
            target_sources(${target_name} PRIVATE ${ADAPTOR_SOURCES})
            message(STATUS "DFM: Added ${interface_name} DBus adaptor sources to target")
        endif()
    else()
        message(WARNING "DFM: ${interface_name} DBus interface file not found: ${DBUS_INTERFACE_FILE}")
    endif()
endfunction()

message(STATUS "DFM: Service configuration module loaded") 
