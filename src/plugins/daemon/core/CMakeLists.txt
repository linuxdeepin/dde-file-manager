cmake_minimum_required(VERSION 3.10)

project(dfmdaemon-core-plugin)

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
find_package(Qt6 COMPONENTS
    Core
    DBus
    Concurrent
    REQUIRED)

find_package(Dtk6 COMPONENTS Core REQUIRED)
find_package(dfm6-search REQUIRED)

# DBus: DeviceManager
# qt6_generate_dbus_interface(
#    devicemanagerdbus.h
#    ${DeviceManager_XML}
#    OPTIONS -M -S
# )
qt6_add_dbus_adaptor(SRC_FILES ${DeviceManager_XML}
    devicemanagerdbus.h DeviceManagerDBus)

# DBus: OperationsStackManager
# qt6_generate_dbus_interface(
#    operationsstackmanagerdbus.h
#    ${OperationsStackManager_XML}
#    OPTIONS -M -S
# )
qt6_add_dbus_adaptor(SRC_FILES ${OperationsStackManager_XML}
    operationsstackmanagerdbus.h OperationsStackManagerDbus)

# use TextIndex interface
qt6_add_dbus_interface(SRC_FILES
    "${CMAKE_SOURCE_DIR}/assets/dbus/org.deepin.Filemanager.TextIndex.xml"
    textindex_interface
)

add_library(${PROJECT_NAME}
    SHARED
    ${SRC_FILES}
)

set_target_properties(${PROJECT_NAME} PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${DFM_BUILD_PLUGIN_DAEMON_DIR})

target_link_libraries(${PROJECT_NAME}
    DFM6::base
    DFM6::framework
    Qt6::Core
    Qt6::DBus
    Qt6::Concurrent
    Dtk6::Core
    dfm6-search
)

#install library file
install(TARGETS
    ${PROJECT_NAME}
    LIBRARY
    DESTINATION
    ${DFM_PLUGIN_FILEMANAGER_CORE_DIR}
)
