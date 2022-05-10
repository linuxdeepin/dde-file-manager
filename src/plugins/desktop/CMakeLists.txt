cmake_minimum_required(VERSION 3.10)

# add sub dir for business plugins
include_directories(
    ${DESKTOP_SERVICE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
    )

add_subdirectory(ddplugin-core)
add_subdirectory(ddplugin-background)
add_subdirectory(ddplugin-canvas)
add_subdirectory(ddplugin-wallpapersetting)
add_subdirectory(ddplugin-dbusregister)
#add_subdirectory(ddplugin-grandsearchdaemon)
#add_subdirectory(ddplugin-organizer)
