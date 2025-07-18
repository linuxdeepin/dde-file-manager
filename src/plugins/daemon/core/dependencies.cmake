function(dfm_setup_daemon_core_dependencies target_name)
    # Apply default plugin configuration first
    dfm_apply_default_plugin_config(${target_name})
    
    # Set DBus XML file paths
    set(DeviceManager_XML ${DFM_DBUS_XML_DIR}/org.deepin.Filemanager.Daemon.DeviceManager.xml)
    set(OperationsStackManager_XML ${DFM_DBUS_XML_DIR}/org.deepin.Filemanager.Daemon.OperationsStackManager.xml)
    
    # Find required packages
    find_package(Qt6 COMPONENTS
        Core
        DBus
        Concurrent
        REQUIRED)
    find_package(Dtk6 COMPONENTS Core REQUIRED)
    find_package(dfm6-search REQUIRED)
    
    # Add DBus adaptors and interfaces
    set(DBUS_SOURCES "")
    qt6_add_dbus_adaptor(DBUS_SOURCES ${DeviceManager_XML}
        devicemanagerdbus.h DeviceManagerDBus)
    qt6_add_dbus_adaptor(DBUS_SOURCES ${OperationsStackManager_XML}
        operationsstackmanagerdbus.h OperationsStackManagerDbus)
    
    # Add DBus interface for TextIndex
    qt6_add_dbus_interface(DBUS_SOURCES
        "${DFM_DBUS_XML_DIR}/org.deepin.Filemanager.TextIndex.xml"
        textindex_interface
    )
    
    # Add DBus sources to target
    target_sources(${target_name} PRIVATE ${DBUS_SOURCES})
    
    # Link required libraries
    target_link_libraries(${target_name} PRIVATE
        Qt6::Core
        Qt6::DBus
        Qt6::Concurrent
        Dtk6::Core
        dfm6-search
    )
endfunction() 
