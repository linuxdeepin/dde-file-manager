cmake_minimum_required(VERSION 3.10)

project(daemonplugin-core)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(DeviceManager_XML ${DFM_DBUS_XML_DIR}/org.deepin.Filemanager.Daemon.DeviceManager.xml)
set(OperationsStackManager_XML ${DFM_DBUS_XML_DIR}/org.deepin.Filemanager.Daemon.OperationsStackManager.xml)

FILE(GLOB_RECURSE SRC_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.json"
    "${DeviceManager_XML}"
    "${OperationsStackManager_XML}"
    )
# TODO: tag manager
find_package(Qt5 COMPONENTS
    Core
    DBus
    REQUIRED)

# DBus: DeviceManager
qt5_generate_dbus_interface(
   devicemanagerdbus.h
   ${DeviceManager_XML}
   OPTIONS -M -S
)
qt5_add_dbus_adaptor(SRC_FILES ${DeviceManager_XML}
    devicemanagerdbus.h DeviceManagerDBus)

# DBus: OperationsStackManager
qt5_generate_dbus_interface(
   operationsstackmanagerdbus.h
   ${OperationsStackManager_XML}
   OPTIONS -M -S
)
qt5_add_dbus_adaptor(SRC_FILES ${OperationsStackManager_XML}
    operationsstackmanagerdbus.h OperationsStackManagerDbus)

add_library(${PROJECT_NAME}
    SHARED
    ${SRC_FILES}
)

set_target_properties(${PROJECT_NAME} PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${DFM_BUILD_PLUGIN_DAEMON_DIR})

find_package(Qt5 COMPONENTS Core)
target_link_libraries(${PROJECT_NAME}
    DFM::base
    DFM::framework
    Qt5::Core
    Qt5::DBus
)

#install library file
install(TARGETS
    ${PROJECT_NAME}
    LIBRARY
    DESTINATION
    ${DFM_PLUGIN_FILEMANAGER_CORE_DIR}
)
