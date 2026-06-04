function(dfm_setup_daemon_opticalshare_dependencies target_name)
    # Apply default plugin configuration first
    dfm_apply_default_plugin_config(${target_name})

    # Configure config.h file
    configure_file(
        "${DFM_APP_SOURCE_DIR}/config.h.in"
        "${CMAKE_CURRENT_BINARY_DIR}/config.h"
        )

    # Set DBus XML file path
    set(OpticalShare_XML ${DFM_DBUS_XML_DIR}/org.deepin.Filemanager.OpticalShare.xml)

    # Find required Qt6 components
    find_package(Qt6 COMPONENTS
        DBus
        REQUIRED)

    # Add DBus adaptor
    set(DBUS_SOURCES "")
    qt6_add_dbus_adaptor(DBUS_SOURCES ${OpticalShare_XML}
        opticalsharedbus.h OpticalShareDBus)

    # Add DBus sources to target
    target_sources(${target_name} PRIVATE ${DBUS_SOURCES})

    # Add include directories
    target_include_directories(${target_name}
        PRIVATE
        ${CMAKE_CURRENT_BINARY_DIR}
        )

    # Link required libraries
    target_link_libraries(${target_name} PRIVATE
        Qt6::DBus
    )
endfunction()
