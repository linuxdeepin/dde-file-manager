cmake_minimum_required(VERSION 3.10)

project(ddplugin-core)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(EXT_FILES
    ${CMAKE_SOURCE_DIR}/src/plugins/desktop/desktoputils/widgetutil.h
    ${CMAKE_SOURCE_DIR}/src/plugins/desktop/desktoputils/ddpugin_eventinterface_helper.h
    )

set(SCREEN_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/screen/screenqt.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/screen/screenqt.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/screen/screenproxyqt.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/screen/screenproxyqt.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/screen/dbus-private/dbushelper.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/screen/dbus-private/dbushelper.cpp"
    )

if (COMPILE_ON_V2X)
    set(SCREEN_FILES
        ${SCREEN_FILES}
        "${CMAKE_CURRENT_SOURCE_DIR}/screen/dbus-private/dbusdock1.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/screen/dbus-private/dbusdock1.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/screen/dbus-private/dbusdisplay1.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/screen/dbus-private/dbusdisplay1.cpp"
    )
else()
    set(SCREEN_FILES
        ${SCREEN_FILES}
        "${CMAKE_CURRENT_SOURCE_DIR}/screen/dbus-private/dbusdock.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/screen/dbus-private/dbusdock.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/screen/dbus-private/dbusdisplay.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/screen/dbus-private/dbusdisplay.cpp"
    )
endif()

set(CORE_FILES
    frame/basewindow.h
    frame/basewindow.cpp
    frame/windowframe_p.h
    frame/windowframe.h
    frame/windowframe.cpp
    ddplugin_core_global.h
    core.h
    core.cpp
    core.json
    )

set(BIN_NAME dd-core-plugin)

find_package(Qt6 COMPONENTS Core Widgets Gui REQUIRED)

add_library(${BIN_NAME}
    SHARED
    ${CORE_FILES}
    ${SCREEN_FILES}
    ${EXT_FILES}
)

set_target_properties(${BIN_NAME} PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${DFM_BUILD_PLUGIN_DESKTOP_DIR})

target_include_directories(${BIN_NAME} PUBLIC
    ${Qt6Widgets_PRIVATE_INCLUDE_DIRS}
)

target_link_libraries(${BIN_NAME}
    DFM6::base
    DFM6::framework
    Qt6::Core
    Qt6::Widgets
    Qt6::Gui
)

install(TARGETS ${BIN_NAME}
    LIBRARY DESTINATION ${DFM_PLUGIN_DESKTOP_CORE_DIR}
)

INSTALL_DCONFIG("org.deepin.dde.file-manager.desktop.json")
