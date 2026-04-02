cmake_minimum_required(VERSION 3.10)

project(dde-file-manager-extractor)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_definitions(-DDFM_PLUGIN_EXTRACTOR_DIR=\"${DFM_PLUGIN_EXTRACTOR_DIR}\")

# Add subdirectories
add_subdirectory(libextractor)
add_subdirectory(extractor)
add_subdirectory(plugins)

message(STATUS "DFM: dde-file-manager-extractor configured")
