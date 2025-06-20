cmake_minimum_required(VERSION 3.10)

project(dfmplugin-smbbrowser)
set(BIN_NAME dfm-smbbrowser-plugin)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

file(GLOB_RECURSE BROWSER_FILES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.json"
)

find_package(Qt6 COMPONENTS Network REQUIRED)
find_package(PkgConfig REQUIRED)

pkg_check_modules(LibSecret REQUIRED libsecret-1 IMPORTED_TARGET)

add_library(${BIN_NAME}
    SHARED
    ${BROWSER_FILES}
)

set_target_properties(${BIN_NAME} PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${DFM_BUILD_PLUGIN_FILEMANAGER_DIR})

target_link_libraries(
    ${BIN_NAME}
    Qt6::Network
    DFM6::base
    DFM6::framework
    PkgConfig::LibSecret
)

#install library file
install(TARGETS
    ${BIN_NAME}
    LIBRARY
    DESTINATION
    ${DFM_PLUGIN_FILEMANAGER_CORE_DIR}
)
