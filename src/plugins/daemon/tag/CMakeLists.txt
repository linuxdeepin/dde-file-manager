cmake_minimum_required(VERSION 3.10)

project(dfmdaemon-tag-plugin)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(TagManager_XML ${DFM_DBUS_XML_DIR}/org.deepin.Filemanager.Daemon.TagManager.xml)

FILE(GLOB_RECURSE SRC_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.json"
    "${TagManager_XML}"
    )

find_package(Qt6 COMPONENTS
    DBus
    Sql
    REQUIRED)

# qt6_generate_dbus_interface(
#    tagmanagerdbus.h
#    ${TagManager_XML}
#    OPTIONS -M -S
# )

qt6_add_dbus_adaptor(SRC_FILES ${TagManager_XML}
    tagmanagerdbus.h TagManagerDBus)

add_library(${PROJECT_NAME}
    SHARED
    ${SRC_FILES}
)

set_target_properties(${PROJECT_NAME} PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${DFM_BUILD_PLUGIN_DAEMON_DIR})

target_link_libraries(${PROJECT_NAME}
    DFM6::base
    DFM6::framework
    Qt6::DBus
    Qt6::Sql
)

#install library file
install(TARGETS
    ${PROJECT_NAME}
    LIBRARY
    DESTINATION
    ${DFM_PLUGIN_FILEMANAGER_CORE_DIR}
)
