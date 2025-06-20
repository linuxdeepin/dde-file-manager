cmake_minimum_required(VERSION 3.10)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

project(test-ddplugin-core)

# UT文件
file(GLOB_RECURSE UT_CXX_FILE
    "${CMAKE_CURRENT_SOURCE_DIR}/*")

set(DESKTOP_PLUGIN_SRC_DIR "${DESKTOP_PLUGIN_DIR}/core/ddplugin-core")

file(GLOB SRC_FILES
    "${DESKTOP_PLUGIN_SRC_DIR}/*.h"
    "${DESKTOP_PLUGIN_SRC_DIR}/*.cpp"
    "${DESKTOP_PLUGIN_SRC_DIR}/frame/*.h"
    "${DESKTOP_PLUGIN_SRC_DIR}/frame/*.cpp"
    "${DESKTOP_PLUGIN_DIR}/desktoputils/*"
)

set(SCREEN_FILES
    "${DESKTOP_PLUGIN_SRC_DIR}/screen/screenqt.h"
    "${DESKTOP_PLUGIN_SRC_DIR}/screen/screenqt.cpp"
    "${DESKTOP_PLUGIN_SRC_DIR}/screen/screenproxyqt.h"
    "${DESKTOP_PLUGIN_SRC_DIR}/screen/screenproxyqt.cpp"
    "${DESKTOP_PLUGIN_SRC_DIR}/screen/dbus-private/dbushelper.h"
    "${DESKTOP_PLUGIN_SRC_DIR}/screen/dbus-private/dbushelper.cpp"
    )

if (COMPLIE_ON_V23)
    set(SCREEN_FILES
        ${SCREEN_FILES}
        "${DESKTOP_PLUGIN_SRC_DIR}/screen/dbus-private/dbusdock1.h"
        "${DESKTOP_PLUGIN_SRC_DIR}/screen/dbus-private/dbusdock1.cpp"
        "${DESKTOP_PLUGIN_SRC_DIR}/screen/dbus-private/dbusdisplay1.h"
        "${DESKTOP_PLUGIN_SRC_DIR}/screen/dbus-private/dbusdisplay1.cpp"
    )
else ()
    set(SCREEN_FILES
        ${SCREEN_FILES}
        "${DESKTOP_PLUGIN_SRC_DIR}/screen/dbus-private/dbusdock.h"
        "${DESKTOP_PLUGIN_SRC_DIR}/screen/dbus-private/dbusdock.cpp"
        "${DESKTOP_PLUGIN_SRC_DIR}/screen/dbus-private/dbusdisplay.h"
        "${DESKTOP_PLUGIN_SRC_DIR}/screen/dbus-private/dbusdisplay.cpp"
    )
endif()

add_executable(${PROJECT_NAME}
    ${UT_CXX_FILE}
    ${CPP_STUB_SRC}
    ${SRC_FILES}
    ${SCREEN_FILES}
)

find_package(Dtk COMPONENTS Widget REQUIRED)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${DESKTOP_PLUGIN_DIR}
    ${DESKTOP_PLUGIN_SRC_DIR}
    ${Qt5Widgets_PRIVATE_INCLUDE_DIRS}
)

target_link_libraries(${PROJECT_NAME}
    DFM::base
    DFM::framework
)

add_test(
  NAME ddplugin-core
  COMMAND $<TARGET_FILE:${PROJECT_NAME}>
)
