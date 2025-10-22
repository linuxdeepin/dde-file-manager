cmake_minimum_required(VERSION 3.10)

project(ddplugin-core)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(EXT_FILES
    ${CMAKE_SOURCE_DIR}/src/plugins/desktop/desktoputils/widgetutil.h
    ${CMAKE_SOURCE_DIR}/src/plugins/desktop/desktoputils/ddplugin_eventinterface_helper.h
    )

set(SCREEN_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/screen/screenqt.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/screen/screenqt.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/screen/screenproxyqt.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/screen/screenproxyqt.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/screen/dbus-private/dbushelper.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/screen/dbus-private/dbushelper.cpp"
    )

set(SCREEN_FILES
    ${SCREEN_FILES}
    "${CMAKE_CURRENT_SOURCE_DIR}/screen/dbus-private/dbusdock1.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/screen/dbus-private/dbusdock1.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/screen/dbus-private/dbusdisplay1.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/screen/dbus-private/dbusdisplay1.cpp"
)

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

# Include plugin configuration
include(DFMPluginConfig)

add_library(${BIN_NAME}
    SHARED
    ${CORE_FILES}
    ${SCREEN_FILES}
    ${EXT_FILES}
)

find_package(Qt6 REQUIRED COMPONENTS Core Gui)
target_link_libraries(${BIN_NAME} 
    PUBLIC
    Qt6::Core
    Qt6::Gui
)
if(Qt6_VERSION VERSION_GREATER_EQUAL 6.10)
    find_package(Qt6 REQUIRED COMPONENTS GuiPrivate)
    target_link_libraries(${BIN_NAME} PRIVATE Qt6::GuiPrivate)
endif()

set_target_properties(${BIN_NAME} PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${DFM_BUILD_PLUGIN_DESKTOP_DIR})

# Include shared dependencies configuration
include(${CMAKE_CURRENT_SOURCE_DIR}/dependencies.cmake)

# Apply plugin specific configuration
dfm_setup_desktop_core_dependencies(${BIN_NAME})

install(TARGETS ${BIN_NAME}
    LIBRARY DESTINATION ${DFM_PLUGIN_DESKTOP_CORE_DIR}
)

INSTALL_DCONFIG("org.deepin.dde.file-manager.desktop.json")
