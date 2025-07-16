# dependencies.cmake - Dependencies configuration for textindex service
# This file defines the specific dependencies and configuration for the text index service

cmake_minimum_required(VERSION 3.10)

# Function to setup textindex service dependencies
function(dfm_setup_textindex_dependencies target_name)
    message(STATUS "DFM: Setting up textindex service dependencies for: ${target_name}")
    
    # Find required packages
    find_package(PkgConfig REQUIRED)
    find_package(Qt6 REQUIRED COMPONENTS Core DBus Gui)
    find_package(Dtk6 COMPONENTS Core)
    find_package(dfm6-search REQUIRED)
    
    # Find system dependencies using pkg-config
    pkg_check_modules(Lucene REQUIRED IMPORTED_TARGET liblucene++ liblucene++-contrib)
    pkg_check_modules(Docparser REQUIRED IMPORTED_TARGET docparser)
    pkg_check_modules(GLIB REQUIRED glib-2.0)
    
    # Apply default service configuration first (this provides DFM6::base, Qt6::Core, Qt6::DBus)
    dfm_apply_default_service_config(${target_name})
    
    # Add textindex-specific dependencies
    target_link_libraries(${target_name} PRIVATE
        Qt6::Gui
        Dtk6::Core
        dfm6-search
        ${GLIB_LIBRARIES}
        PkgConfig::Lucene
        PkgConfig::Docparser
    )
    
    # Add textindex-specific include directories
    target_include_directories(${target_name} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${GLIB_INCLUDE_DIRS}
        ${DFM_PROJECT_ROOT}/include
        ${DFM_PROJECT_ROOT}/3rdparty
    )
    # Add fulltext sources - this ensures textindex can find fulltext/chineseanalyzer.h
    set(FULL_TEXT_PATH "${DFM_PROJECT_ROOT}/3rdparty/fulltext")
    file(GLOB FULLTEXT_SOURCES 
        "${FULL_TEXT_PATH}/*.cpp"
        "${FULL_TEXT_PATH}/*.h"
    )
    
    if(FULLTEXT_SOURCES)
        target_sources(${target_name} PRIVATE ${FULLTEXT_SOURCES})
        message(STATUS "DFM: Added fulltext sources to textindex target")
    endif()
    
    # Setup DBus interface generation
    dfm_setup_textindex_dbus_interfaces(${target_name})
    
    message(STATUS "DFM: Text index service dependencies configured successfully")
endfunction()

# Function to setup textindex-specific DBus interfaces
function(dfm_setup_textindex_dbus_interfaces target_name)
    message(STATUS "DFM: Setting up textindex DBus interfaces for: ${target_name}")
    
    # Define the DBus interface file path using DFM_ASSETS_DIR
    set(DBUS_INTERFACE_FILE "${DFM_ASSETS_DIR}/dbus/org.deepin.Filemanager.TextIndex.xml")
    
    # Check if the DBus interface file exists
    if(EXISTS "${DBUS_INTERFACE_FILE}")
        message(STATUS "DFM: Found TextIndex DBus interface file: ${DBUS_INTERFACE_FILE}")
        
        # Generate DBus interface and adaptor
        # qt6_generate_dbus_interface(
        #     textindexdbus.h
        #     ${DBUS_INTERFACE_FILE}
        #     OPTIONS -M -S
        # )
        
        qt6_add_dbus_adaptor(ADAPTOR_SOURCES ${DBUS_INTERFACE_FILE}
            textindexdbus.h TextIndexDBus)
        
        # Add generated sources to target
        if(ADAPTOR_SOURCES)
            target_sources(${target_name} PRIVATE ${ADAPTOR_SOURCES})
            message(STATUS "DFM: Added TextIndex DBus adaptor sources to target")
        endif()
    else()
        message(WARNING "DFM: TextIndex DBus interface file not found: ${DBUS_INTERFACE_FILE}")
    endif()
endfunction()

message(STATUS "DFM: Text index service dependencies configuration loaded") 
