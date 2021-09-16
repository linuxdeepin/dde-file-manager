cmake_minimum_required(VERSION 3.1.0)

# add sub dir for business plugins

include_directories(${FILEMANAGER_SERVICE_DIR})

add_subdirectory(dfmplugin-core)
add_subdirectory(dfmplugin-recent)
