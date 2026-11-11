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
    pkg_check_modules(MINIZIP REQUIRED minizip IMPORTED_TARGET)
    
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
    # PUBLIC: Dependencies exposed in public API (headers)
    # PRIVATE: Internal implementation dependencies
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
            ${DFM_EXTRA_LIBRARIES}
        PRIVATE
            poppler-cpp
            ${LIBHEIF_LIBRARIES}
            libappimage
            PkgConfig::MINIZIP
    )

    # Minimized window restore on Wayland: xdg-shell has no un-minimize request,
    # and xdg-activation cannot help because it only trusts tokens minted by a
    # window the user is interacting with - which a minimized one never is.
    # zwlr_foreign_toplevel_manager_v1 is what window switchers use instead, and
    # it is a wlroots protocol rather than a compositor specific one. Optional:
    # without it the library still builds and activation falls back to a plain
    # request.
    find_package(Qt${QT_VERSION_MAJOR} QUIET COMPONENTS WaylandClient)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(WLR_PROTOCOLS QUIET wlr-protocols)
    # pkg_check_modules does not export pkgdatadir, so read it directly. It is
    # the only reliable pointer to where the protocol definitions live.
    pkg_get_variable(WLR_PROTOCOLS_PKGDATADIR wlr-protocols pkgdatadir)
    set(DFM_FOREIGN_TOPLEVEL_XML
        ${WLR_PROTOCOLS_PKGDATADIR}/unstable/wlr-foreign-toplevel-management-unstable-v1.xml)
    if(TARGET Qt${QT_VERSION_MAJOR}::WaylandClient
            AND WLR_PROTOCOLS_FOUND
            AND EXISTS "${DFM_FOREIGN_TOPLEVEL_XML}")
        qt6_generate_wayland_protocol_client_sources(${target_name}
            FILES ${DFM_FOREIGN_TOPLEVEL_XML})
        target_link_libraries(${target_name} PRIVATE Qt${QT_VERSION_MAJOR}::WaylandClient)
        target_compile_definitions(${target_name} PRIVATE DFM_WITH_FOREIGN_TOPLEVEL)
    else()
        message(STATUS "DFM: foreign-toplevel protocol unavailable, minimized window restore is disabled")
    endif()

    # Space-key preview window transition: the compositor plays the open/close
    # animation around a rectangle attached to an xdg-activation token. That is
    # a Treeland feature, so it depends on treeland-protocols, and the raw
    # wl_surface access needs Qt's private Wayland headers
    # (qt6-wayland-private-dev), exposed by the QtWaylandClientPrivate target.
    # Independent of the foreign-toplevel block above: either can be absent.
    find_package(TreelandProtocols QUIET)
    pkg_get_variable(WAYLAND_PROTOCOLS_DATADIR wayland-protocols pkgdatadir)
    set(DFM_WINDOW_TRANSITION_XML
        ${TREELAND_PROTOCOLS_DATA_DIR}/treeland-window-transition-unstable-v1.xml)
    set(DFM_XDG_ACTIVATION_XML
        ${WAYLAND_PROTOCOLS_DATADIR}/staging/xdg-activation/xdg-activation-v1.xml)
    # Both XML files are inputs to the generator, which aborts the whole build
    # when one is missing, so each of them has to be probed. wayland-protocols
    # is only reachable through pkg-config: no Qt package depends on the file
    # the transition protocol needs.
    if(TARGET Qt${QT_VERSION_MAJOR}::WaylandClientPrivate
            AND EXISTS "${DFM_WINDOW_TRANSITION_XML}"
            AND EXISTS "${DFM_XDG_ACTIVATION_XML}")
        qt6_generate_wayland_protocol_client_sources(${target_name}
            FILES
                ${DFM_XDG_ACTIVATION_XML}
                ${DFM_WINDOW_TRANSITION_XML})
        target_link_libraries(${target_name} PRIVATE Qt${QT_VERSION_MAJOR}::WaylandClientPrivate)
        target_compile_definitions(${target_name} PRIVATE DFM_WITH_TREELAND_TRANSITION)
    else()
        message(STATUS "DFM: window transition protocol unavailable, preview transition is disabled")
    endif()

    # File dialog dock-icon hiding: the dialog sets the skip-dock-preview bit
    # of treeland-dde-shell-v1 on its surface, which removes the window from
    # the foreign toplevel lists the dock taskbar consumes. Another Treeland
    # protocol from the package the transition block above probes, and the
    # raw wl_surface access reuses the private Wayland headers that block
    # links. Independent of both blocks above: each can be absent.
    set(DFM_DDE_SHELL_XML
        ${TREELAND_PROTOCOLS_DATA_DIR}/treeland-dde-shell-v1.xml)
    if(TARGET Qt${QT_VERSION_MAJOR}::WaylandClientPrivate
            AND EXISTS "${DFM_DDE_SHELL_XML}")
        # NO_INCLUDE_CORE_ONLY: the protocol references wl_callback, whose
        # interface symbol only wayland-client.h (not the -core split) declares.
        qt6_generate_wayland_protocol_client_sources(${target_name}
            NO_INCLUDE_CORE_ONLY
            FILES ${DFM_DDE_SHELL_XML})
        target_link_libraries(${target_name} PRIVATE
            Qt${QT_VERSION_MAJOR}::WaylandClient
            Qt${QT_VERSION_MAJOR}::WaylandClientPrivate)
        target_compile_definitions(${target_name} PRIVATE DFM_WITH_DDE_SHELL)
    else()
        message(STATUS "DFM: dde-shell protocol unavailable, file dialog dock-icon hiding is disabled")
    endif()

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

message(STATUS "DFM: Library configuration module loaded") 
