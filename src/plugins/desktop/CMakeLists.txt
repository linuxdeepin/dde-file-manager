cmake_minimum_required(VERSION 3.10)

# add sub dir for business plugins
include_directories(${DESKTOP_SERVICE_DIR})

add_subdirectory(ddplugin-core)
add_subdirectory(ddplugin-dbusregister)
