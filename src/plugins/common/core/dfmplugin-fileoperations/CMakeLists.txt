cmake_minimum_required(VERSION 3.10)

project(dfmplugin-fileoperations)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

FILE(GLOB_RECURSE FILEOPERATIONS_FILES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.json"
    )
find_package(Dtk COMPONENTS Widget REQUIRED)
find_package(Qt5 COMPONENTS Core REQUIRED)
find_package(Qt5 COMPONENTS DBus REQUIRED)
find_package(PkgConfig REQUIRED)

pkg_check_modules(zlib REQUIRED zlib IMPORTED_TARGET)

# generate dbus interface
qt5_add_dbus_interface(FILEOPERATIONS_FILES
    ${DFM_DBUS_XML_DIR}/org.deepin.filemanager.server.OperationsStackManager.xml
    operationsstackmanager_interface)

add_library(${PROJECT_NAME}
    SHARED
    ${FILEOPERATIONS_FILES}
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
