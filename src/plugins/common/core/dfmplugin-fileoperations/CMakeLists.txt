cmake_minimum_required(VERSION 3.10)

project(dfmplugin-fileoperations)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

FILE(GLOB_RECURSE FILEOPERATIONS_FILES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.json"
    )

find_package(PkgConfig REQUIRED)

pkg_check_modules(zlib REQUIRED zlib IMPORTED_TARGET)

# generate dbus interface
if(DFM_BUILD_WITH_QT6)
    qt6_add_dbus_interface(FILEOPERATIONS_FILES
        ${DFM_DBUS_XML_DIR}/org.deepin.filemanager.server.OperationsStackManager.xml
        operationsstackmanager_interface)
else()
    qt5_add_dbus_interface(FILEOPERATIONS_FILES
        ${DFM_DBUS_XML_DIR}/org.deepin.filemanager.server.OperationsStackManager.xml
        operationsstackmanager_interface)
endif()

add_library(${PROJECT_NAME}
    SHARED
    ${FILEOPERATIONS_FILES}
)

set_target_properties(${PROJECT_NAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ../../)

target_link_libraries(${PROJECT_NAME}
    DFM::base
    DFM::framework
    ${DtkWidget_LIBRARIES}
    PkgConfig::zlib
)

#install library file
install(TARGETS
    ${PROJECT_NAME}
    LIBRARY
    DESTINATION
    ${DFM_PLUGIN_COMMON_CORE_DIR}
)
INSTALL_DCONFIG("org.deepin.dde.file-manager.operations.json")
