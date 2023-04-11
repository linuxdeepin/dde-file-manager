cmake_minimum_required(VERSION 3.10)

project(serverplugin-filemanager1)

set(CMAKE_INCLUDE_CURRENT_DIR ON)


FILE(GLOB_RECURSE SRC_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.json"
    "${DFM_DBUS_XML_DIR}/org.freedesktop.FileManager1.xml"
    )

find_package(Qt5 COMPONENTS
    Core
    DBus
    REQUIRED)

qt5_generate_dbus_interface(
    filemanager1dbus.h
    ${DFM_DBUS_XML_DIR}/org.freedesktop.FileManager1.xml
    OPTIONS -M -S
)

qt5_add_dbus_adaptor(SRC_FILES ${DFM_DBUS_XML_DIR}/org.freedesktop.FileManager1.xml
    filemanager1dbus.h FileManager1DBus)

add_library(${PROJECT_NAME}
    SHARED
    ${SRC_FILES}
)

set_target_properties(${PROJECT_NAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ../../)

target_link_libraries(${PROJECT_NAME}
    DFM::base
    DFM::framework
    Qt5::Core
    Qt5::DBus
)

#install library file
install(TARGETS
    ${PROJECT_NAME}
    LIBRARY
    DESTINATION
    ${DFM_PLUGIN_SERVER_EDGE_DIR}
)

# install dbus
install(FILES org.freedesktop.FileManager.service DESTINATION
    ${CMAKE_INSTALL_PREFIX}/share/dbus-1/services)
