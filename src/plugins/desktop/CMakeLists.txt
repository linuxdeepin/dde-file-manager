cmake_minimum_required(VERSION 3.10)

# add sub dir for business plugins
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
)

if(NOT DEFINED DESKTOP_ENABLE_ORGANIZER)
    if (COMPLIE_ON_V23)
    set(DESKTOP_ENABLE_ORGANIZER ON)
    else()
    set(DESKTOP_ENABLE_ORGANIZER OFF)
    endif()
endif()

add_subdirectory(core/ddplugin-core)
add_subdirectory(core/ddplugin-canvas)
add_subdirectory(core/ddplugin-dbusregister)
add_subdirectory(ddplugin-background)
add_subdirectory(ddplugin-wallpapersetting)
add_subdirectory(ddplugin-grandsearchdaemon)

message(STATUS "DESKTOP_ENABLE_ORGANIZER: ${DESKTOP_ENABLE_ORGANIZER}")
if (DESKTOP_ENABLE_ORGANIZER)
add_subdirectory(ddplugin-organizer)
endif()

