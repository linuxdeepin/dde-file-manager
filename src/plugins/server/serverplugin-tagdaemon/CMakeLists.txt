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

find_package(Qt${QT_VERSION_MAJOR} COMPONENTS
    DBus
    Sql
    REQUIRED)

#qt5_generate_dbus_interface(
#    tagmanagerdbus.h
#    ${TagManager_XML}
#    OPTIONS -M -S
#)

if(DFM_BUILD_WITH_QT6)
    qt_add_dbus_adaptor(SRC_FILES ${TagManager_XML}
        tagmanagerdbus.h TagManagerDBus)
else()
    qt5_add_dbus_adaptor(SRC_FILES ${TagManager_XML}
        tagmanagerdbus.h TagManagerDBus)
endif()

add_library(${PROJECT_NAME}
    SHARED
    ${SRC_FILES}
)

set_target_properties(${PROJECT_NAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ../../)

target_link_libraries(${PROJECT_NAME}
    DFM::base
    DFM::framework
    Qt${QT_VERSION_MAJOR}::DBus
    Qt${QT_VERSION_MAJOR}::Sql
)

#install library file
install(TARGETS
    ${PROJECT_NAME}
    LIBRARY
    DESTINATION
    ${DFM_PLUGIN_SERVER_EDGE_DIR}
)
