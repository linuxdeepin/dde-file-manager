cmake_minimum_required(VERSION 3.10)

set(test_name "test-ddplugin-core")
set(plugin_path "${DFM_SOURCE_DIR}/plugins/desktop/ddplugin-core")

# Find test files
file(GLOB_RECURSE UT_CXX_FILE FILES_MATCHING PATTERN "*.cpp" "*.h")

# Define source files exactly as in the original plugin CMakeLists.txt
set(EXT_FILES
    ${DFM_SOURCE_DIR}/plugins/desktop/desktoputils/widgetutil.h
    ${DFM_SOURCE_DIR}/plugins/desktop/desktoputils/ddpugin_eventinterface_helper.h
)

set(SCREEN_FILES
    "${plugin_path}/screen/screenqt.h"
    "${plugin_path}/screen/screenqt.cpp"
    "${plugin_path}/screen/screenproxyqt.h"
    "${plugin_path}/screen/screenproxyqt.cpp"
    "${plugin_path}/screen/dbus-private/dbushelper.h"
    "${plugin_path}/screen/dbus-private/dbushelper.cpp"
)

set(SCREEN_FILES
    ${SCREEN_FILES}
    "${plugin_path}/screen/dbus-private/dbusdock1.h"
    "${plugin_path}/screen/dbus-private/dbusdock1.cpp"
    "${plugin_path}/screen/dbus-private/dbusdisplay1.h"
    "${plugin_path}/screen/dbus-private/dbusdisplay1.cpp"
)

set(CORE_FILES
    ${plugin_path}/frame/basewindow.h
    ${plugin_path}/frame/basewindow.cpp
    ${plugin_path}/frame/windowframe_p.h
    ${plugin_path}/frame/windowframe.h
    ${plugin_path}/frame/windowframe.cpp
    ${plugin_path}/ddplugin_core_global.h
    ${plugin_path}/core.h
    ${plugin_path}/core.cpp
)

# Create test executable with precise file control
dfm_create_test_executable(${test_name}
    SOURCES ${UT_CXX_FILE} ${CORE_FILES} ${SCREEN_FILES} ${EXT_FILES}
)

# Apply plugin-specific configuration using shared dependencies
dfm_configure_plugin_dependencies(${test_name} "ddplugin-core" ${plugin_path})

# Include necessary paths to resolve relative includes
target_include_directories(${test_name} PRIVATE 
    "${plugin_path}"
    "${DFM_SOURCE_DIR}/plugins/desktop"
    "${CMAKE_CURRENT_SOURCE_DIR}"
)

message(STATUS "DFM: Created enhanced desktop core plugin test: ${test_name}") 
