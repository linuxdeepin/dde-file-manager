# Dependencies configuration for ddplugin-canvas
# This file contains the dependency setup for the desktop canvas plugin

function(dfm_setup_desktop_canvas_dependencies target_name)
    message(STATUS "DFM: Setting up desktop canvas plugin dependencies for: ${target_name}")
    
    # Apply default plugin configuration first
    dfm_apply_default_plugin_config(${target_name})
    
    # Find required packages
    find_package(Qt6 COMPONENTS Core Widgets Gui Svg REQUIRED)
    
    # Link libraries
    target_link_libraries(${target_name} PRIVATE
        Qt6::Core
        Qt6::Widgets
        Qt6::Gui
        Qt6::Svg
    )
    
    # Include directories
    target_include_directories(${target_name} PRIVATE
        ${Qt6Widgets_PRIVATE_INCLUDE_DIRS}
    )
    
    message(STATUS "DFM: Desktop canvas plugin dependencies configured successfully for: ${target_name}")
endfunction() 