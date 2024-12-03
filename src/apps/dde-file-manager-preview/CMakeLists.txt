cmake_minimum_required(VERSION 3.10)

if(NOT DEFINED DFM_PLUGIN_PREVIEW_DIR)
    set(DFM_PLUGIN_PREVIEW_DIR ${DFM_PLUGIN_DIR}/previews)
endif()
add_definitions(-DPLUGINDIR=\"${DFM_PLUGIN_PREVIEW_DIR}\")

add_subdirectory(filepreview)
add_subdirectory(libdfm-preview)
add_subdirectory(pluginpreviews)
