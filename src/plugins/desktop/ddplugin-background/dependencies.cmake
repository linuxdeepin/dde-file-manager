# Dependencies configuration for ddplugin-background
# This file contains the dependency setup for the desktop background plugin

function(dfm_setup_desktop_background_dependencies target_name)
    message(STATUS "DFM: Setting up desktop background plugin dependencies for: ${target_name}")
    
    # Apply default plugin configuration first
    dfm_apply_default_plugin_config(${target_name})
    
    # Find required packages
    find_package(Qt6 COMPONENTS Core Widgets Gui Svg REQUIRED)
    
    # Link libraries
    target_link_libraries(${target_name} PRIVATE
        Qt6::Core
        Qt6::Widgets
        Qt6::Gui
    )
    
    # Include directories
    target_include_directories(${target_name} PRIVATE
        ${Qt6Widgets_PRIVATE_INCLUDE_DIRS}
    )
    
    message(STATUS "DFM: Desktop background plugin dependencies configured successfully for: ${target_name}")
endfunction() 
