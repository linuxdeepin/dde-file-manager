function(dfm_setup_daemon_vault_dependencies target_name)
    # Apply default plugin configuration first
    dfm_apply_default_plugin_config(${target_name})
    
    # Set DBus XML file path
    set(VaultManager_XML ${DFM_DBUS_XML_DIR}/org.deepin.Filemanager.Daemon.VaultManager.xml)
    
    # Find required Qt6 components
    find_package(Qt6 COMPONENTS DBus REQUIRED)
    
    # Find PkgConfig and libsecret-1
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(secret REQUIRED libsecret-1 IMPORTED_TARGET)
    
    # Add definitions
    add_definitions(-DQT_NO_KEYWORDS)
    
    # Add DBus adaptor
    set(DBUS_SOURCES "")
    qt6_add_dbus_adaptor(DBUS_SOURCES ${VaultManager_XML}
        dbus/vaultmanagerdbus.h VaultManagerDBus)
    
    # Add DBus sources to target
    target_sources(${target_name} PRIVATE ${DBUS_SOURCES})
    
    # Link required libraries
    target_link_libraries(${target_name} PRIVATE
        Qt6::DBus
        PkgConfig::secret
    )
endfunction() 
