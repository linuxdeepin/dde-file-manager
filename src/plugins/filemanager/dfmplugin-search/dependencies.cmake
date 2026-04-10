# dependencies.cmake - Dependencies configuration for dfmplugin-search
# This file defines the specific dependencies and configuration for the search plugin

cmake_minimum_required(VERSION 3.10)

# Function to setup search plugin dependencies
function(dfm_setup_search_dependencies target_name)
    message(STATUS "DFM: Setting up search plugin dependencies for: ${target_name}")
    
    # Find required packages
    find_package(Qt6 REQUIRED COMPONENTS DBus)
    find_package(dfm6-search REQUIRED)
    find_package(PkgConfig REQUIRED)
    
    # Find system dependencies using pkg-config
    pkg_check_modules(Lucene REQUIRED IMPORTED_TARGET liblucene++ liblucene++-contrib)
    pkg_check_modules(GLIB REQUIRED glib-2.0)
    
    # Apply default plugin configuration first
    dfm_apply_default_plugin_config(${target_name})
    
    # Add search-specific dependencies
    target_link_libraries(${target_name} PRIVATE
        Qt6::DBus
        ${GLIB_LIBRARIES}
        PkgConfig::Lucene
        dfm6-search
    )
    
    # Add search-specific include directories
    target_include_directories(${target_name} PRIVATE
        ${CMAKE_SOURCE_DIR}/3rdparty
        ${GLIB_INCLUDE_DIRS}
        ${dfm6-search_INCLUDE_DIR}
    )
    
    # Handle platform-specific definitions
    if (${CMAKE_SYSTEM_PROCESSOR} MATCHES "aarch64")
        target_compile_definitions(${target_name} PRIVATE ARM_PROCESSOR)
        message(STATUS "DFM: Added ARM_PROCESSOR definition for aarch64")
    endif()
    
    # Setup DBus interface generation
    dfm_setup_search_dbus_interfaces(${target_name})
    
    message(STATUS "DFM: Search plugin dependencies configured successfully")
endfunction()

# Function to setup search-specific DBus interfaces
function(dfm_setup_search_dbus_interfaces target_name)
    message(STATUS "DFM: Setting up search DBus interfaces for: ${target_name}")
    
    set(TEXTINDEX_DBUS_INTERFACE_FILE "${DFM_ASSETS_DIR}/dbus/org.deepin.Filemanager.TextIndex.xml")
    set(OCRINDEX_DBUS_INTERFACE_FILE "${DFM_ASSETS_DIR}/dbus/org.deepin.Filemanager.OcrIndex.xml")
    set(INTERFACE_SOURCES)

    if(EXISTS "${TEXTINDEX_DBUS_INTERFACE_FILE}")
        message(STATUS "DFM: Found TextIndex DBus interface file: ${TEXTINDEX_DBUS_INTERFACE_FILE}")
        qt_add_dbus_interface(INTERFACE_SOURCES ${TEXTINDEX_DBUS_INTERFACE_FILE} textindex_interface)
    else()
        message(WARNING "DFM: TextIndex DBus interface file not found: ${TEXTINDEX_DBUS_INTERFACE_FILE}")
    endif()

    if(EXISTS "${OCRINDEX_DBUS_INTERFACE_FILE}")
        message(STATUS "DFM: Found OcrIndex DBus interface file: ${OCRINDEX_DBUS_INTERFACE_FILE}")
        qt_add_dbus_interface(INTERFACE_SOURCES ${OCRINDEX_DBUS_INTERFACE_FILE} ocrindex_interface)
    else()
        message(WARNING "DFM: OcrIndex DBus interface file not found: ${OCRINDEX_DBUS_INTERFACE_FILE}")
    endif()

    if(INTERFACE_SOURCES)
        target_sources(${target_name} PRIVATE ${INTERFACE_SOURCES})
        message(STATUS "DFM: Added search DBus interface sources to target")
    endif()
endfunction()

message(STATUS "DFM: Search plugin dependencies configuration loaded") 
