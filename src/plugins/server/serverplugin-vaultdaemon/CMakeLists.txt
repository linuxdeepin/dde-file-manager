cmake_minimum_required(VERSION 3.10)

project(serverplugin-vaultdaemon)

find_package(Qt5 COMPONENTS DBus REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(secret REQUIRED libsecret-1 IMPORTED_TARGET)
add_definitions(-DQT_NO_KEYWORDS)

FILE(GLOB_RECURSE SRC_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.json"
    "${DFM_DBUS_XML_DIR}/org.deepin.filemanager.server.VaultManager.xml"
    )

# DBus: VaultManager
#qt5_generate_dbus_interface(
#    dbus/vaultmanagerdbus.h
#    ${DFM_DBUS_XML_DIR}/org.deepin.filemanager.server.VaultManager.xml
#    OPTIONS -M -S
#)
qt5_add_dbus_adaptor(SRC_FILES ${DFM_DBUS_XML_DIR}/org.deepin.filemanager.server.VaultManager.xml
    dbus/vaultmanagerdbus.h VaultManagerDBus)

add_library(${PROJECT_NAME}
    SHARED
    ${SRC_FILES}
    )

set_target_properties(${PROJECT_NAME} PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${DFM_BUILD_PLUGIN_SERVER_DIR})

target_link_libraries(${PROJECT_NAME}
    DFM::framework
    DFM::base
    Qt5::DBus
    PkgConfig::secret
    )

#install library file
install(TARGETS
    ${PROJECT_NAME}
    LIBRARY
    DESTINATION
    ${DFM_PLUGIN_SERVER_EDGE_DIR}
    )
