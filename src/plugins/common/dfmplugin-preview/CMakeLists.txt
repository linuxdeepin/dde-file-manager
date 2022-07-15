cmake_minimum_required(VERSION 3.10)

if(NOT DEFINED DFM_PLUGIN_PREVIEW_DIR)
    set(DFM_PLUGIN_PREVIEW_DIR ${DFM_PLUGIN_DIR}/previews)
endif()

add_subdirectory(filepreview)
add_subdirectory(pluginpreviews)
