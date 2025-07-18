function(dfm_setup_daemon_recent_dependencies target_name)
    # Apply default plugin configuration first
    dfm_apply_default_plugin_config(${target_name})
    
    # Set DBus XML file path
    set(RecentManager_XML ${DFM_DBUS_XML_DIR}/org.deepin.Filemanager.Daemon.RecentManager.xml)
    
    # Find required Qt6 components
    find_package(Qt6 COMPONENTS
        Core
        DBus
        REQUIRED)
    
    # Add DBus adaptor
    set(DBUS_SOURCES "")
    qt6_add_dbus_adaptor(DBUS_SOURCES ${RecentManager_XML}
        recentmanagerdbus.h RecentManagerDBus)
    
    # Add DBus sources to target
    target_sources(${target_name} PRIVATE ${DBUS_SOURCES})
    
    # Link required libraries
    target_link_libraries(${target_name} PRIVATE
        Qt6::Core
        Qt6::DBus
    )
endfunction() 
