function(dfm_setup_filedialog_core_dependencies target_name)
    # Apply default plugin configuration first
    dfm_apply_default_plugin_config(${target_name})
    
    # Find required Qt6 components
    find_package(Qt6 COMPONENTS Core DBus Widgets REQUIRED)
    
    # Add DBus adaptors
    set(DBUS_SOURCES "")
    set(DFM_FILEDIALOG_CORE_DIR ${DFM_SOURCE_DIR}/plugins/filedialog/core)
    qt6_add_dbus_adaptor(DBUS_SOURCES ${DFM_FILEDIALOG_CORE_DIR}/dbus/com.deepin.filemanager.filedialog.xml
        ${DFM_FILEDIALOG_CORE_DIR}/dbus/filedialoghandledbus.h FileDialogHandleDBus)
    qt6_add_dbus_adaptor(DBUS_SOURCES ${DFM_FILEDIALOG_CORE_DIR}/dbus/com.deepin.filemanager.filedialogmanager.xml
        ${DFM_FILEDIALOG_CORE_DIR}/dbus/filedialogmanagerdbus.h FileDialogManagerDBus)
    
    # Add DBus sources to target
    target_sources(${target_name} PRIVATE ${DBUS_SOURCES})
    
    # Link required libraries
    target_link_libraries(${target_name} PRIVATE
        Qt6::Core
        Qt6::DBus
        Qt6::Widgets
    )
endfunction() 
