cmake_minimum_required(VERSION 3.1.0)

# add sub dir for business plugins
include_directories(${DESKTOP_SERVICE_DIR})

add_subdirectory(ddplugin-core)
