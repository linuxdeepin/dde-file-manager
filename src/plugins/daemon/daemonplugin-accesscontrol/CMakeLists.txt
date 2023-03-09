cmake_minimum_required(VERSION 3.10)

project(daemonplugin-accesscontrol)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

FILE(GLOB FILEOPERATIONS_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.json"
    "${CMAKE_CURRENT_SOURCE_DIR}/*/*.policy"
    )


add_library(${PROJECT_NAME}
    SHARED
    ${FILEOPERATIONS_FILES}
)

set_target_properties(${PROJECT_NAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ../../)

find_package(PkgConfig REQUIRED)

pkg_search_module(dfm-io
    REQUIRED
    dfm-io
    IMPORTED_TARGET
)
pkg_search_module(dfm-mount
    REQUIRED
    dfm-mount
    IMPORTED_TARGET
)

pkg_search_module(crypt
    REQUIRED
    libcryptsetup
    IMPORTED_TARGET
)
pkg_check_modules(PolkitAgent
    REQUIRED
    polkit-agent-1
    IMPORTED_TARGET
)
pkg_check_modules(PolkitQt5
    REQUIRED
    polkit-qt5-1
    IMPORTED_TARGET
)
find_package(Qt5 COMPONENTS
    DBus
    REQUIRED
)

target_link_libraries(${PROJECT_NAME}
    DFM::framework
    DFM::base
    PkgConfig::dfm-io
    PkgConfig::dfm-mount
    PkgConfig::crypt
    PkgConfig::PolkitAgent
    PkgConfig::PolkitQt5
    Qt5::DBus
    ${DtkWidget_LIBRARIES}
)

#install library file
install(TARGETS
    ${PROJECT_NAME}
    LIBRARY
    DESTINATION
    ${DFM_PLUGIN_DAEMON_EDGE_DIR}
)

set(PolicyDir "${CMAKE_INSTALL_PREFIX}/share/polkit-1/actions")
install(FILES polkit/com.deepin.filemanager.daemon.accesscontrol.policy DESTINATION ${PolicyDir})
