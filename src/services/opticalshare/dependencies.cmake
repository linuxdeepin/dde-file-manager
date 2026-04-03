# dependencies.cmake - Dependencies configuration for opticalshare service

cmake_minimum_required(VERSION 3.10)

function(dfm_setup_opticalshare_dependencies target_name)
    message(STATUS "DFM: Setting up opticalshare service dependencies for: ${target_name}")

    find_package(Qt6 REQUIRED COMPONENTS DBus)

    dfm_apply_default_service_config(${target_name})

    target_link_libraries(${target_name} PRIVATE
        Qt6::DBus
    )

    dfm_setup_opticalshare_dbus_interfaces(${target_name})

    message(STATUS "DFM: Opticalshare service dependencies configured successfully")
endfunction()

function(dfm_setup_opticalshare_dbus_interfaces target_name)
    message(STATUS "DFM: Setting up opticalshare DBus interfaces for: ${target_name}")

    set(DBUS_INTERFACE_FILE "${DFM_ASSETS_DIR}/dbus/org.deepin.Filemanager.OpticalShare.xml")

    if(EXISTS "${DBUS_INTERFACE_FILE}")
        message(STATUS "DFM: Found OpticalShare DBus interface file: ${DBUS_INTERFACE_FILE}")

        qt6_add_dbus_adaptor(ADAPTOR_SOURCES ${DBUS_INTERFACE_FILE}
            opticalsharedbus.h OpticalShareDBus)

        if(ADAPTOR_SOURCES)
            target_sources(${target_name} PRIVATE ${ADAPTOR_SOURCES})
            message(STATUS "DFM: Added OpticalShare DBus adaptor sources to target")
        endif()
    else()
        message(WARNING "DFM: OpticalShare DBus interface file not found: ${DBUS_INTERFACE_FILE}")
    endif()
endfunction()

message(STATUS "DFM: Opticalshare service dependencies configuration loaded")
