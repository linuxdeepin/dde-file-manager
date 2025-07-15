# DFMPluginConfig.cmake - Plugin configuration management for DDE File Manager
# This module provides centralized plugin dependency configuration

cmake_minimum_required(VERSION 3.10)

# Function to apply default plugin configuration for simple plugins
function(dfm_apply_default_plugin_config target_name)
    message(STATUS "DFM: Applying default plugin configuration for: ${target_name}")
    
    # Default plugin dependencies - covers most common plugin needs
    target_link_libraries(${target_name} PRIVATE
        DFM6::base
        DFM6::framework
    )
    
    message(STATUS "DFM: Default plugin configuration applied successfully")
endfunction()

# Function to setup plugin DBus interfaces
function(dfm_setup_plugin_dbus_interfaces target_name plugin_path)
    message(STATUS "DFM: Setting up DBus interfaces for: ${target_name}")
    
    # Look for DBus interface definitions in the plugin directory
    file(GLOB_RECURSE DBUS_XML_FILES "${plugin_path}/*.xml")
    
    if(DBUS_XML_FILES)
        message(STATUS "DFM: Found DBus XML files: ${DBUS_XML_FILES}")
        
        # Generate DBus interfaces for each XML file found
        foreach(xml_file ${DBUS_XML_FILES})
            get_filename_component(xml_name ${xml_file} NAME_WE)
            set(interface_name "${xml_name}_interface")
            
            message(STATUS "DFM: Generating DBus interface: ${interface_name}")
            qt_add_dbus_interface(INTERFACE_SOURCES ${xml_file} ${interface_name})
        endforeach()
        
        # Add generated sources to target
        if(INTERFACE_SOURCES)
            target_sources(${target_name} PRIVATE ${INTERFACE_SOURCES})
            message(STATUS "DFM: Added DBus interface sources to target")
        endif()
    else()
        message(STATUS "DFM: No DBus XML files found in plugin directory")
    endif()
endfunction()

# Function to include plugin-specific dependencies from dependencies.cmake
function(dfm_include_plugin_dependencies target_name dependencies_file)
    message(STATUS "DFM: Including plugin dependencies from: ${dependencies_file}")
    
    if(EXISTS "${dependencies_file}")
        # Include the dependencies file
        include("${dependencies_file}")
        
        # Extract plugin name from dependencies file path
        get_filename_component(plugin_dir ${dependencies_file} DIRECTORY)
        get_filename_component(plugin_name ${plugin_dir} NAME)
        
        # Try to call plugin-specific setup function
        string(REPLACE "dfmplugin-" "" plugin_short_name ${plugin_name})
        set(setup_function_name "dfm_setup_${plugin_short_name}_dependencies")
        
        if(COMMAND ${setup_function_name})
            message(STATUS "DFM: Calling plugin setup function: ${setup_function_name}")
            cmake_language(CALL ${setup_function_name} ${target_name})
        else()
            message(WARNING "DFM: Plugin setup function not found: ${setup_function_name}")
        endif()
    else()
        message(STATUS "DFM: Dependencies file not found, using default configuration")
        dfm_apply_default_plugin_config(${target_name})
    endif()
endfunction()

# Main function to configure plugin dependencies
function(dfm_configure_plugin_dependencies target_name plugin_name plugin_path)
    message(STATUS "DFM: Configuring plugin dependencies for: ${plugin_name}")
    
    # Check if plugin has custom dependencies file
    set(dependencies_file "${plugin_path}/dependencies.cmake")
    
    if(EXISTS "${dependencies_file}")
        message(STATUS "DFM: Found custom dependencies file for ${plugin_name}")
        dfm_include_plugin_dependencies(${target_name} "${dependencies_file}")
    else()
        message(STATUS "DFM: Using default configuration for ${plugin_name}")
        dfm_apply_default_plugin_config(${target_name})
    endif()
    
    # Setup DBus interfaces if any exist
    dfm_setup_plugin_dbus_interfaces(${target_name} "${plugin_path}")
    
    message(STATUS "DFM: Plugin dependencies configured successfully for: ${plugin_name}")
endfunction()

# Function to register plugin-specific configuration functions
function(dfm_register_plugin_config plugin_name config_function)
    message(STATUS "DFM: Registering plugin configuration: ${plugin_name} -> ${config_function}")
    set_property(GLOBAL PROPERTY "DFM_PLUGIN_CONFIG_${plugin_name}" "${config_function}")
endfunction()

# Function to get registered plugin configuration function
function(dfm_get_plugin_config plugin_name result_var)
    get_property(config_function GLOBAL PROPERTY "DFM_PLUGIN_CONFIG_${plugin_name}")
    set(${result_var} "${config_function}" PARENT_SCOPE)
endfunction()

message(STATUS "DFM: Plugin configuration module loaded") 
