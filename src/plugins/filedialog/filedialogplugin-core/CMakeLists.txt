cmake_minimum_required(VERSION 3.10)

project(filedialogplugin-core)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

FILE(GLOB_RECURSE SRC_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.json"
)

find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core Widgets Gui DBus REQUIRED)

if(DFM_BUILD_WITH_QT6)
    qt_add_dbus_adaptor(SRC_FILES ${CMAKE_CURRENT_SOURCE_DIR}/dbus/com.deepin.filemanager.filedialog.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/dbus/filedialoghandledbus.h FileDialogHandleDBus)
    qt_add_dbus_adaptor(SRC_FILES ${CMAKE_CURRENT_SOURCE_DIR}/dbus/com.deepin.filemanager.filedialogmanager.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/dbus/filedialogmanagerdbus.h FileDialogManagerDBus)
else()
    qt5_add_dbus_adaptor(SRC_FILES ${CMAKE_CURRENT_SOURCE_DIR}/dbus/com.deepin.filemanager.filedialog.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/dbus/filedialoghandledbus.h FileDialogHandleDBus)
    qt5_add_dbus_adaptor(SRC_FILES ${CMAKE_CURRENT_SOURCE_DIR}/dbus/com.deepin.filemanager.filedialogmanager.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/dbus/filedialogmanagerdbus.h FileDialogManagerDBus)
endif()

add_library(${PROJECT_NAME}
    SHARED
    ${SRC_FILES}
)

set_target_properties(${PROJECT_NAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ../../)

if(DFM_BUILD_WITH_QT6)
    target_include_directories(${PROJECT_NAME} PUBLIC
        ${Qt6Widgets_PRIVATE_INCLUDE_DIRS}
    )
else()
    target_include_directories(${PROJECT_NAME} PUBLIC
        ${Qt5Widgets_PRIVATE_INCLUDE_DIRS}
    )
endif()

target_link_libraries(${PROJECT_NAME}
    DFM::base
    DFM::framework
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::DBus
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
