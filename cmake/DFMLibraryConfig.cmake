# DFMLibraryConfig.cmake - Unified library configuration for DDE File Manager
# This module provides centralized dependency configuration for all DFM libraries

cmake_minimum_required(VERSION 3.10)

# Function to configure dfm-base library dependencies and settings
function(dfm_configure_base_library target_name)
    message(STATUS "DFM: Configuring dfm-base library: ${target_name}")
    
    # Find dependencies - Qt6 components
    find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core Widgets Gui Concurrent DBus Sql Network REQUIRED)
    find_package(Dtk${DTK_VERSION_MAJOR} COMPONENTS Core Widget Gui REQUIRED)
    find_package(dfm${DTK_VERSION_MAJOR}-io REQUIRED)
    find_package(dfm${DTK_VERSION_MAJOR}-mount REQUIRED)
    find_package(dfm${DTK_VERSION_MAJOR}-burn REQUIRED)
    find_package(libappimage REQUIRED)
    find_package(PkgConfig REQUIRED)
    
    # System dependencies
    pkg_check_modules(gio REQUIRED gio-unix-2.0 IMPORTED_TARGET)
    pkg_check_modules(mount REQUIRED mount IMPORTED_TARGET)
    pkg_check_modules(LIBHEIF REQUIRED libheif)
    pkg_search_module(X11 REQUIRED x11 IMPORTED_TARGET)
    
    # Qt version specific dependencies
    if(${QT_VERSION_MAJOR} EQUAL "6")
        set(DFM_EXTRA_LIBRARIES "")
        set(DFM_IO_HEADERS ${dfm6-io_INCLUDE_DIR})
        set(DFM_MOUNT_HEADERS ${dfm6-mount_INCLUDE_DIR})
        set(DFM_BURN_HEADERS ${dfm6-burn_INCLUDE_DIR})
    else()
        find_package(KF5Codecs REQUIRED)
        find_package(Qt5 COMPONENTS X11Extras REQUIRED)
        pkg_search_module(gsettings REQUIRED gsettings-qt IMPORTED_TARGET)
        set(DFM_EXTRA_LIBRARIES
            Qt${QT_VERSION_MAJOR}::X11Extras
            PkgConfig::gsettings
            PkgConfig::X11
            KF5::Codecs)
        set(DFM_IO_HEADERS ${dfm-io_INCLUDE_DIR})
        set(DFM_MOUNT_HEADERS ${dfm-mount_INCLUDE_DIR})
        set(DFM_BURN_HEADERS ${dfm-burn_INCLUDE_DIR})
    endif()
    
    # Generate DBus interface for Qt6
    if(${QT_VERSION_MAJOR} EQUAL "6")
        qt6_add_dbus_interface(Qt6App_dbus
            ${DFM_DBUS_XML_DIR}/org.deepin.Filemanager.Daemon.DeviceManager.xml
            devicemanager_interface_qt6)
        target_sources(${target_name} PRIVATE ${Qt6App_dbus})
    else()
        qt5_add_dbus_interface(Qt5App_dbus
            ${DFM_DBUS_XML_DIR}/org.deepin.Filemanager.Daemon.DeviceManager.xml
            devicemanager_interface)
        target_sources(${target_name} PRIVATE ${Qt5App_dbus})
    endif()
    
    # Link libraries
    target_link_libraries(${target_name} 
        PUBLIC
            Qt${QT_VERSION_MAJOR}::Core
            Qt${QT_VERSION_MAJOR}::Widgets
            Qt${QT_VERSION_MAJOR}::Gui
            Qt${QT_VERSION_MAJOR}::Concurrent
            Qt${QT_VERSION_MAJOR}::DBus
            Qt${QT_VERSION_MAJOR}::Sql
            Qt${QT_VERSION_MAJOR}::Network
            Dtk${DTK_VERSION_MAJOR}::Core
            Dtk${DTK_VERSION_MAJOR}::Widget
            Dtk${DTK_VERSION_MAJOR}::Gui
            dfm${DTK_VERSION_MAJOR}-io
            dfm${DTK_VERSION_MAJOR}-mount
            dfm${DTK_VERSION_MAJOR}-burn
            PkgConfig::mount
            PkgConfig::gio
            PkgConfig::X11
            poppler-cpp
            ${LIBHEIF_LIBRARIES}
            ${DFM_EXTRA_LIBRARIES}  
            libappimage
    )
    
    # Include directories
    target_include_directories(${target_name} 
        PUBLIC
            "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>"
            "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..>"
            "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
            ${DFM_IO_HEADERS}
            ${DFM_MOUNT_HEADERS}
            ${DFM_BURN_HEADERS}
    )
    
    # Compile definitions
    set(ShareDir ${CMAKE_INSTALL_PREFIX}/share/dde-file-manager)
    target_compile_definitions(${target_name}
        PRIVATE APPSHAREDIR="${ShareDir}"
        PRIVATE DFM_BASE_INTERNAL_USE=1
    )

    message(STATUS "DFM: dfm-base library configured successfully")
endfunction()

# Function to configure dfm-framework library dependencies and settings
function(dfm_configure_framework_library target_name)
    message(STATUS "DFM: Configuring dfm-framework library: ${target_name}")
    
    # Qt6 dependencies
    find_package(Qt6 COMPONENTS 
        Core 
        Concurrent
        REQUIRED
    )
    find_package(Dtk6 COMPONENTS Core REQUIRED)
    find_package(PkgConfig REQUIRED)
    
    # Link libraries
    target_link_libraries(${target_name} PUBLIC
        Qt6::Core
        Qt6::Concurrent
        Dtk6::Core
    )
    
    # Include directories
    target_include_directories(${target_name} PUBLIC
        ${PROJECT_SOURCE_DIR}/include
    )
    
    # Compile definitions
    target_compile_definitions(${target_name}
        PRIVATE VERSION="${CMAKE_PROJECT_VERSION}"
        PRIVATE DPF_INTERNAL_USE=1
    )
    
    message(STATUS "DFM: dfm-framework library configured successfully")
endfunction()

# Function to configure dfm-extension library dependencies and settings
function(dfm_configure_extension_library target_name)
    message(STATUS "DFM: Configuring dfm-extension library: ${target_name}")
    
    # Include directories
    target_include_directories(${target_name} PUBLIC
        ${PROJECT_SOURCE_DIR}/include
    )
    
    # Compile definitions
    target_compile_definitions(${target_name} PRIVATE VERSION="${CMAKE_PROJECT_VERSION}")
    
    message(STATUS "DFM: dfm-extension library configured successfully")
endfunction()

# Function to get library dependencies for testing
function(dfm_get_library_test_dependencies lib_name result_var)
    if(lib_name STREQUAL "dfm-base")
        set(${result_var} "Qt6::Core;Qt6::Widgets;Qt6::Gui;Qt6::Concurrent;Qt6::DBus;Qt6::Sql;Qt6::Network;Dtk6::Core;Dtk6::Widget;Dtk6::Gui;dfm6-io;dfm6-mount;dfm6-burn;PkgConfig::mount;PkgConfig::gio;PkgConfig::X11;poppler-cpp;${LIBHEIF_LIBRARIES};libappimage" PARENT_SCOPE)
    elseif(lib_name STREQUAL "dfm-framework")
        set(${result_var} "Qt6::Core;Qt6::Concurrent;Dtk6::Core;${CMAKE_DL_LIBS}" PARENT_SCOPE)
    elseif(lib_name STREQUAL "dfm-extension")
        set(${result_var} "Qt6::Core;${CMAKE_DL_LIBS}" PARENT_SCOPE)
    else()
        message(FATAL_ERROR "DFM: Unknown library name: ${lib_name}")
    endif()
endfunction()

message(STATUS "DFM: Library configuration module loaded") 
