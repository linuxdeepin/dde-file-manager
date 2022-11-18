cmake_minimum_required(VERSION 3.10)

set(GlobalFiles ${CMAKE_CURRENT_SOURCE_DIR}/preview_plugin_global.h)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory(image-preview)
add_subdirectory(music-preview)
add_subdirectory(pdf-preview)
add_subdirectory(text-preview)
add_subdirectory(video-preview)
add_subdirectory(dciicon-preview)
