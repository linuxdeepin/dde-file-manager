cmake_minimum_required(VERSION 3.1.0)

# add sub dir for business plugins

# TODO(zhangs): if a file not service that should be hidden int a service!
include_directories(${FILEMANAGER_SERVICE_DIR})
include_directories(${FILEMANAGER_SERVICE_DIR}/window)

add_subdirectory(dfmplugin-core)
add_subdirectory(dfmplugin-recent)
