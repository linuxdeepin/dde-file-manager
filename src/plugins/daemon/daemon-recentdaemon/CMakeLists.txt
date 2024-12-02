cmake_minimum_required(VERSION 3.10)

project(serverplugin-recentdaemon)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(RecentManager_XML ${DFM_DBUS_XML_DIR}/org.deepin.Filemanager.Daemon.RecentManager.xml)

FILE(GLOB_RECURSE SRC_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.json"
    "${RecentManager_XML}"
    )

find_package(Qt5 COMPONENTS
    Core
    DBus
    REQUIRED)

qt5_generate_dbus_interface(
    recentmanagerdbus.h
    ${RecentManager_XML}
    OPTIONS -M -S
)

qt5_add_dbus_adaptor(SRC_FILES ${RecentManager_XML}
    recentmanagerdbus.h RecentManagerDBus)

add_library(${PROJECT_NAME}
    SHARED
    ${SRC_FILES}
)

set_target_properties(${PROJECT_NAME} PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${DFM_BUILD_PLUGIN_DAEMON_DIR})

target_link_libraries(${PROJECT_NAME}
    DFM::base
    DFM::framework
    Qt5::Core
    Qt5::DBus
)

install(TARGETS
    ${PROJECT_NAME}
    LIBRARY
    DESTINATION
    ${DFM_PLUGIN_FILEMANAGER_CORE_DIR}
)
