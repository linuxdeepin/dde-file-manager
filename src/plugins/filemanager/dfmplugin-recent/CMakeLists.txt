cmake_minimum_required(VERSION 3.10)

project(dfmplugin-recent)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

FILE(GLOB_RECURSE RECENT_FILES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.json"
)

find_package(Qt6 COMPONENTS Xml REQUIRED)
find_package(Qt6 COMPONENTS DBus REQUIRED)

# generate dbus interface
qt6_add_dbus_interface(RECENT_FILES
    ${DFM_DBUS_XML_DIR}/org.deepin.Filemanager.Daemon.RecentManager.xml
    recentmanager_interface)

add_library(${PROJECT_NAME}
    SHARED
    ${RECENT_FILES}
)

set_target_properties(${PROJECT_NAME} PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${DFM_BUILD_PLUGIN_FILEMANAGER_DIR})

target_link_libraries(${PROJECT_NAME}
    DFM6::base
    DFM6::framework
    Qt6::Xml
    Qt6::DBus
)

#install library file
install(TARGETS
    ${PROJECT_NAME}
    LIBRARY
    DESTINATION
    ${DFM_PLUGIN_FILEMANAGER_CORE_DIR}
)
