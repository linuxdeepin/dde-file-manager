cmake_minimum_required(VERSION 3.10)

project(dfmdaemon-filemanager1-plugin)

configure_file(
    "${APP_SOURCE_DIR}/config.h.in"
    "${CMAKE_CURRENT_BINARY_DIR}/config.h"
    )

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(FILEMANAGER1_XML ${DFM_DBUS_XML_DIR}/org.freedesktop.FileManager1.xml)

FILE(GLOB_RECURSE SRC_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.json"
    "${FILEMANAGER1_XML}"
    )

find_package(Qt6 COMPONENTS
    DBus
    REQUIRED)

qt6_add_dbus_adaptor(SRC_FILES ${FILEMANAGER1_XML}
    filemanager1dbus.h FileManager1DBus)

add_library(${PROJECT_NAME}
    SHARED
    ${SRC_FILES}
)

set_target_properties(${PROJECT_NAME} PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${DFM_BUILD_PLUGIN_DAEMON_DIR})

target_include_directories(${PROJECT_NAME}
    PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR}
    )

target_link_libraries(${PROJECT_NAME}
    DFM6::base
    DFM6::framework
    Qt6::DBus
)

#install library file
install(TARGETS
    ${PROJECT_NAME}
    LIBRARY
    DESTINATION
    ${DFM_PLUGIN_FILEMANAGER_CORE_DIR}
)

# install dbus
install(FILES org.freedesktop.FileManager.service DESTINATION
    ${CMAKE_INSTALL_PREFIX}/share/dbus-1/services)
