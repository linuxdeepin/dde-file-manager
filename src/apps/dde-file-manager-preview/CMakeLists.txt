cmake_minimum_required(VERSION 3.10)


add_definitions(-DDFM_PLUGIN_PREVIEW_DIR=\"${DFM_PLUGIN_PREVIEW_DIR}\")

add_subdirectory(filepreview)
add_subdirectory(libdfm-preview)
add_subdirectory(pluginpreviews)
