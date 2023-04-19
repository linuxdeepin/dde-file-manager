cmake_minimum_required(VERSION 3.10)

project(dfmplugin-tag)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

FILE(GLOB_RECURSE TAG_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.json"
    )

find_package(Dtk COMPONENTS Widget REQUIRED)
find_package(Qt5 COMPONENTS Core REQUIRED)
find_package(Qt5 COMPONENTS DBus REQUIRED)

# generate dbus interface
qt5_add_dbus_interface(TAG_FILES
    ${DFM_DBUS_XML_DIR}/org.deepin.filemanager.server.TagManager.xml
    tagmanager_interface)

add_library(${PROJECT_NAME}
    SHARED
    ${TAG_FILES}
)

set_target_properties(${PROJECT_NAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ../../)

target_include_directories(${PROJECT_NAME}
    PUBLIC
        ${DtkWidget_INCLUDE_DIRS}
)

target_link_libraries(${PROJECT_NAME}
    Qt5::Core
    Qt5::DBus
    DFM::base
    DFM::framework
    ${DtkWidget_LIBRARIES}
)

#install library file
install(TARGETS
    ${PROJECT_NAME}
    LIBRARY
    DESTINATION
    ${DFM_PLUGIN_COMMON_EDGE_DIR}
)
