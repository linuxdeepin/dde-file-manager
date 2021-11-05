cmake_minimum_required(VERSION 3.1.0)

# add sub dir for business plugins

include_directories(${COMMON_SERVICE_DIR}/device)
include_directories(${COMMON_SERVICE_DIR}/dialog)
add_subdirectory(dfmsplugin-interfaces)
