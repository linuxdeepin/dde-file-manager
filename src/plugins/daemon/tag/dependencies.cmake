function(dfm_setup_daemon_tag_dependencies target_name)
    # Apply default plugin configuration first
    dfm_apply_default_plugin_config(${target_name})
    
    # Set DBus XML file path
    set(TagManager_XML ${DFM_DBUS_XML_DIR}/org.deepin.Filemanager.Daemon.TagManager.xml)
    
    # Find required Qt6 components
    find_package(Qt6 COMPONENTS
        DBus
        Sql
        REQUIRED)
    
    # Add DBus adaptor
    set(DBUS_SOURCES "")
    qt6_add_dbus_adaptor(DBUS_SOURCES ${TagManager_XML}
        tagmanagerdbus.h TagManagerDBus)
    
    # Add DBus sources to target
    target_sources(${target_name} PRIVATE ${DBUS_SOURCES})
    
    # Link required libraries
    target_link_libraries(${target_name} PRIVATE
        Qt6::DBus
        Qt6::Sql
    )
endfunction() 
