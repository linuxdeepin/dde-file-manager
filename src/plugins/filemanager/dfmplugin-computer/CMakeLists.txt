cmake_minimum_required(VERSION 3.10)

project(dfmplugin-computer)
set(BIN_NAME dfm-computer-plugin)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

FILE(GLOB_RECURSE COMPUTER_FILES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.json"
    )

find_package(PkgConfig REQUIRED)
pkg_check_modules(Secret REQUIRED libsecret-1)

find_package(Qt6 COMPONENTS Core)
add_library(${BIN_NAME}
    SHARED
    ${COMPUTER_FILES}
)

set_target_properties(${BIN_NAME} PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${DFM_BUILD_PLUGIN_FILEMANAGER_DIR})

target_include_directories(${BIN_NAME}
    PUBLIC
        ${Secret_INCLUDE_DIRS}
)

target_link_libraries(${BIN_NAME}
    DFM6::base
    DFM6::framework
    ${Secret_LIBRARIES}
)

#install library file
install(TARGETS
    ${BIN_NAME}
    LIBRARY
    DESTINATION
    ${DFM_PLUGIN_FILEMANAGER_CORE_DIR}
)

set(AppEntryInstallDir "${CMAKE_INSTALL_PREFIX}/share/dde-file-manager/extensions/appEntry")
install(FILES .readme DESTINATION ${AppEntryInstallDir})

# install dconfig files.
INSTALL_DCONFIG("org.deepin.dde.file-manager.computer.json")
INSTALL_DCONFIG("org.deepin.dde.file-manager.mount.json")
