cmake_minimum_required(VERSION 3.10)

project(serverplugin-vaultdaemon)

find_package(Qt${QT_VERSION_MAJOR} COMPONENTS DBus REQUIRED)
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
if(DFM_BUILD_WITH_QT6)
    qt_add_dbus_adaptor(SRC_FILES ${DFM_DBUS_XML_DIR}/org.deepin.filemanager.server.VaultManager.xml
        dbus/vaultmanagerdbus.h VaultManagerDBus)
else()
    qt5_add_dbus_adaptor(SRC_FILES ${DFM_DBUS_XML_DIR}/org.deepin.filemanager.server.VaultManager.xml
        dbus/vaultmanagerdbus.h VaultManagerDBus)
endif()

add_library(${PROJECT_NAME}
    SHARED
    ${SRC_FILES}
    )

set_target_properties(${PROJECT_NAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ../../)

target_link_libraries(${PROJECT_NAME}
    DFM::framework
    DFM::base
    Qt${QT_VERSION_MAJOR}::DBus
    )

#install library file
install(TARGETS
    ${PROJECT_NAME}
    LIBRARY
    DESTINATION
    ${DFM_PLUGIN_SERVER_EDGE_DIR}
    )
