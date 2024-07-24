cmake_minimum_required(VERSION 3.10)

project(dfmplugin-tag)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

FILE(GLOB_RECURSE TAG_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.json"
    )

find_package(Qt${QT_VERSION_MAJOR} COMPONENTS DBus REQUIRED)

# generate dbus interface
if(DFM_BUILD_WITH_QT6)
    qt_add_dbus_interface(TAG_FILES
        ${DFM_DBUS_XML_DIR}/org.deepin.filemanager.server.TagManager.xml
        tagmanager_interface)
else()
    qt5_add_dbus_interface(TAG_FILES
        ${DFM_DBUS_XML_DIR}/org.deepin.filemanager.server.TagManager.xml
        tagmanager_interface)
endif()

add_library(${PROJECT_NAME}
    SHARED
    ${TAG_FILES}
)

set_target_properties(${PROJECT_NAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ../../)

target_link_libraries(${PROJECT_NAME}
    Qt${QT_VERSION_MAJOR}::DBus
    DFM::base
    DFM::framework
)

#install library file
install(TARGETS
    ${PROJECT_NAME}
    LIBRARY
    DESTINATION
    ${DFM_PLUGIN_COMMON_EDGE_DIR}
)
