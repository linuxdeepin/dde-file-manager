cmake_minimum_required(VERSION 3.10)

project(filedialog-core-plugin)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

FILE(GLOB_RECURSE SRC_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.json"
)

find_package(Qt6 COMPONENTS DBus Widgets REQUIRED)

qt6_add_dbus_adaptor(SRC_FILES ${CMAKE_CURRENT_SOURCE_DIR}/dbus/com.deepin.filemanager.filedialog.xml
    ${CMAKE_CURRENT_SOURCE_DIR}/dbus/filedialoghandledbus.h FileDialogHandleDBus)
qt6_add_dbus_adaptor(SRC_FILES ${CMAKE_CURRENT_SOURCE_DIR}/dbus/com.deepin.filemanager.filedialogmanager.xml
    ${CMAKE_CURRENT_SOURCE_DIR}/dbus/filedialogmanagerdbus.h FileDialogManagerDBus)

add_library(${PROJECT_NAME}
    SHARED
    ${SRC_FILES}
)

set_target_properties(${PROJECT_NAME} PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${DFM_BUILD_PLUGIN_FILEDIALOG_DIR})

target_link_libraries(${PROJECT_NAME}
    DFM6::base
    DFM6::framework
    Qt6::DBus
)

#install library file
install(TARGETS
    ${PROJECT_NAME}
    LIBRARY
    DESTINATION
    ${DFM_PLUGIN_FILEMANAGER_CORE_DIR}
)

# install dbus
install(FILES dbus/com.deepin.filemanager.filedialog.xml DESTINATION share/dbus-1/interfaces)
install(FILES dbus/com.deepin.filemanager.filedialogmanager.xml DESTINATION share/dbus-1/interfaces)
