cmake_minimum_required(VERSION 3.10)

project(serverplugin-tagdaemon)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(TagManager_XML ${DFM_DBUS_XML_DIR}/org.deepin.filemanager.server.TagManager.xml)

FILE(GLOB_RECURSE SRC_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.json"
    "${TagManager_XML}"
    )

find_package(Qt5 COMPONENTS
    Core
    DBus
    Sql
    REQUIRED)

#qt5_generate_dbus_interface(
#    tagmanagerdbus.h
#    ${TagManager_XML}
#    OPTIONS -M -S
#)

qt5_add_dbus_adaptor(SRC_FILES ${TagManager_XML}
    tagmanagerdbus.h TagManagerDBus)

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
    Qt5::Sql
)

#install library file
install(TARGETS
    ${PROJECT_NAME}
    LIBRARY
    DESTINATION
    ${DFM_PLUGIN_SERVER_EDGE_DIR}
)
