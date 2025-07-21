# Dependencies configuration for ddplugin-core
# This file contains the dependency setup for the desktop core plugin

function(dfm_setup_desktop_core_dependencies target_name)
    message(STATUS "DFM: Setting up desktop core plugin dependencies for: ${target_name}")
    
    # Apply default plugin configuration first
    dfm_apply_default_plugin_config(${target_name})
    
    # Find required packages
    find_package(Qt6 COMPONENTS Core Widgets Gui REQUIRED)
    find_package(DDEShell REQUIRED)
    
    # Link libraries
    target_link_libraries(${target_name} PRIVATE
        Qt6::Core
        Qt6::Widgets
        Qt6::Gui
        Dde::Shell
    )
    
    # Include directories
    target_include_directories(${target_name} PRIVATE
        ${Qt6Widgets_PRIVATE_INCLUDE_DIRS}
    )
    
    message(STATUS "DFM: Desktop core plugin dependencies configured successfully for: ${target_name}")
endfunction() 