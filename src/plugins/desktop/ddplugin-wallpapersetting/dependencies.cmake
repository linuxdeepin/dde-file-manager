# Dependencies configuration for ddplugin-wallpapersetting
# This file contains the dependency setup for the desktop wallpapersetting plugin

function(dfm_setup_desktop_wallpapersetting_dependencies target_name)
    message(STATUS "DFM: Setting up desktop wallpapersetting plugin dependencies for: ${target_name}")
    
    # Apply default plugin configuration first
    dfm_apply_default_plugin_config(${target_name})
    
    # Find required packages
    find_package(Qt6 COMPONENTS Core Svg REQUIRED)
    find_package(Dtk6 COMPONENTS Widget REQUIRED)
    find_package(PkgConfig REQUIRED)
    
    # Check for xcb using pkg-config
    pkg_check_modules(xcb REQUIRED xcb IMPORTED_TARGET)
    
    # Link libraries
    target_link_libraries(${target_name} PRIVATE
        Qt6::Svg
        PkgConfig::xcb
    )
    
    # Include directories
    target_include_directories(${target_name} PRIVATE
        ${Qt6Widgets_PRIVATE_INCLUDE_DIRS}
    )
    
    message(STATUS "DFM: Desktop wallpapersetting plugin dependencies configured successfully for: ${target_name}")
endfunction() 