cmake_minimum_required(VERSION 3.10)

set(DESKTOP_PLUGIN_DIR "${PROJECT_SOURCE_PATH}/plugins/desktop")

# add sub dir for business plugins
add_subdirectory(core/ddplugin-core)
add_subdirectory(core/ddplugin-canvas)
add_subdirectory(ddplugin-background)
add_subdirectory(ddplugin-wallpapersetting)
#add_subdirectory(ddplugin-grandsearchdaemon)
add_subdirectory(ddplugin-organizer)
