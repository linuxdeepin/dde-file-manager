cmake_minimum_required(VERSION 3.10)

project(daemonplugin-filemanager1)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(FILEMANGER1_XML ${DFM_DBUS_XML_DIR}/org.freedesktop.FileManager1.xml)

FILE(GLOB_RECURSE SRC_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.json"
    "${FILEMANGER1_XML}"
    )

find_package(Qt5 COMPONENTS
    DBus
    REQUIRED)

qt5_add_dbus_adaptor(SRC_FILES ${FILEMANGER1_XML}
    filemanager1dbus.h FileManager1DBus)

add_library(${PROJECT_NAME}
    SHARED
    ${SRC_FILES}
)

set_target_properties(${PROJECT_NAME} PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${DFM_BUILD_PLUGIN_SERVER_DIR})

target_link_libraries(${PROJECT_NAME}
    DFM::base
    DFM::framework
    Qt5::DBus
)

#install library file
install(TARGETS
    ${PROJECT_NAME}
    LIBRARY
    DESTINATION
    ${DFM_PLUGIN_DAEMON_EDGE_DIR}
)

# install dbus
install(FILES org.freedesktop.FileManager.service DESTINATION
    ${CMAKE_INSTALL_PREFIX}/share/dbus-1/services)
