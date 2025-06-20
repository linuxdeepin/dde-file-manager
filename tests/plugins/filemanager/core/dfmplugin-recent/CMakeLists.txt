cmake_minimum_required(VERSION 3.10)

project(test-dfmplugin-recent)

set(PluginPath ${PROJECT_SOURCE_PATH}/plugins/filemanager/core/dfmplugin-recent)

# UI files
file(GLOB_RECURSE UT_CXX_FILE
    FILES_MATCHING PATTERN "*.cpp" "*.h")
file(GLOB_RECURSE SRC_FILES
    FILES_MATCHING PATTERN "${PluginPath}/*.cpp" "${PluginPath}/*.h")

find_package(Dtk COMPONENTS Widget REQUIRED)
find_package(Qt5 COMPONENTS DBus REQUIRED)

# generate dbus interface
qt5_add_dbus_interface(SRC_FILES
    ${DFM_DBUS_XML_DIR}/org.deepin.filemanager.server.RecentManager.xml
    recentmanager_interface)


add_executable(${PROJECT_NAME}
    ${SRC_FILES}
    ${UT_CXX_FILE}
    ${CPP_STUB_SRC}
)

target_include_directories(${PROJECT_NAME} PRIVATE
    "${PluginPath}")
target_link_libraries(${PROJECT_NAME} PRIVATE
    DFM::base
    DFM::framework
    Qt5::DBus
    ${DtkWidget_LIBRARIES}
)

add_test(
    NAME recent
    COMMAND $<TARGET_FILE:${PROJECT_NAME}>
)
