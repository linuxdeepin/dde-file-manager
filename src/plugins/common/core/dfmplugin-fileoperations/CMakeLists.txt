cmake_minimum_required(VERSION 3.10)

project(dfmplugin-fileoperations)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

FILE(GLOB_RECURSE FILEOPERATIONS_FILES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.json"
    )

include(dfmplugin-fileoperations-qt5.cmake)

# Qt6
if (DFM_BUILD_WITH_QT6)
include(dfmplugin-fileoperations-qt6.cmake)
endif()

# generate dbus interface
qt5_add_dbus_interface(FILEOPERATIONS_FILES
    ${DFM_DBUS_XML_DIR}/org.deepin.Filemanager.Daemon.OperationsStackManager.xml
    operationsstackmanager_interface)

INSTALL_DCONFIG("org.deepin.dde.file-manager.operations.json")
