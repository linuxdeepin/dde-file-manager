cmake_minimum_required(VERSION 3.10)

# add sub dir for business plugins
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
)

add_subdirectory(core/ddplugin-core)
add_subdirectory(core/ddplugin-canvas)
add_subdirectory(ddplugin-background)
add_subdirectory(ddplugin-wallpapersetting)
add_subdirectory(ddplugin-organizer)
