cmake_minimum_required(VERSION 3.10)

project(ddplugin-core)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(EXT_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/../../desktoputils/widgetutil.h
    ${CMAKE_CURRENT_SOURCE_DIR}/../../desktoputils/ddpugin_eventinterface_helper.h
    )

set(SCREEN_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/screen/screenqt.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/screen/screenqt.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/screen/screenproxyqt.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/screen/screenproxyqt.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/screen/dbus-private/dbushelper.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/screen/dbus-private/dbushelper.cpp"
    )

if (COMPLIE_ON_V23)
    set(SCREEN_FILES
        ${SCREEN_FILES}
        "${CMAKE_CURRENT_SOURCE_DIR}/screen/dbus-private/dbusdock1.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/screen/dbus-private/dbusdock1.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/screen/dbus-private/dbusdisplay1.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/screen/dbus-private/dbusdisplay1.cpp"
    )
else ()
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

add_library(${PROJECT_NAME}
    SHARED
    ${CORE_FILES}
    ${SCREEN_FILES}
    ${EXT_FILES}
)

set_target_properties(${PROJECT_NAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ../../)
target_link_libraries(${PROJECT_NAME}
    DFM::base
    DFM::framework
    )

#install library file
install(TARGETS
    ${PROJECT_NAME}
    LIBRARY
    DESTINATION
    ${DFM_PLUGIN_DESKTOP_CORE_DIR}
)

INSTALL_DCONFIG("org.deepin.dde.file-manager.desktop.json")
