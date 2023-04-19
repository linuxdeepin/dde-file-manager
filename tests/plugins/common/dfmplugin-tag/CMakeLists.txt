cmake_minimum_required(VERSION 3.10)

project(test-dfmplugin-tag)

set(PluginPath ${PROJECT_SOURCE_PATH}/plugins/common/dfmplugin-tag/)

# UT文件
file(GLOB_RECURSE UT_CXX_FILE
    FILES_MATCHING PATTERN "*.cpp" "*.h")
file(GLOB_RECURSE SRC_FILES
    FILES_MATCHING PATTERN "${PluginPath}/*.cpp" "${PluginPath}/*.h"
    )

find_package(Dtk COMPONENTS Widget REQUIRED)
find_package(Qt5 COMPONENTS DBus REQUIRED)

# generate dbus interface
qt5_add_dbus_interface(SRC_FILES
    ${DFM_DBUS_XML_DIR}/org.deepin.filemanager.server.TagManager.xml
    tagmanager_interface)

add_executable(${PROJECT_NAME}
    ${SRC_FILES}
    ${UT_CXX_FILE}
    ${CPP_STUB_SRC}
)

target_include_directories(${PROJECT_NAME} PRIVATE
    "${PluginPath}")
target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt5::DBus
    DFM::base
    DFM::framework
    ${DtkWidget_LIBRARIES}
)

add_test(
  NAME tag
  COMMAND $<TARGET_FILE:${PROJECT_NAME}>
)
