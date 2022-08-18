cmake_minimum_required(VERSION 3.10)

project(test-dfmplugin-sidebar)

set(PluginPath ${PROJECT_SOURCE_PATH}/plugins/filemanager/core/dfmplugin-sidebar/)
set(ModelPath ${PluginPath}/treemodels/)
set(ViewPath ${PluginPath}/treeviews/)
set(EventPath ${PluginPath}/events/)
set(UtilsPath ${PluginPath}/utils/)
include_directories(${PluginPath}/treeviews)
include_directories(${PluginPath}/treemodels)
# UT文件
file(GLOB_RECURSE UT_CXX_FILE
    FILES_MATCHING PATTERN "*.cpp" "*.h")
#file(GLOB_RECURSE SRC_FILES
#    FILES_MATCHING PATTERN "${PluginPath}/*.cpp" "${PluginPath}/*.h")
file(GLOB SRC_FILES
    FILES_MATCHING PATTERN "${PluginPath}/sidebar.h" "${PluginPath}/sidebar.cpp"
    FILES_MATCHING PATTERN "${UtilsPath}/*.h" "${UtilsPath}/*.cpp"
    FILES_MATCHING PATTERN "${EventPath}/*.h" "${EventPath}/*.cpp"
    FILES_MATCHING PATTERN "${ModelPath}/sidebarmodel.h" "${ModelPath}/sidebarmodel.cpp"
    FILES_MATCHING PATTERN "${ViewPath}/sidebarview.h" "${ViewPath}/sidebarview.cpp"
    FILES_MATCHING PATTERN "${ViewPath}/sidebaritem.h" "${ViewPath}/sidebaritem.cpp"
    FILES_MATCHING PATTERN "${ViewPath}/sidebaritemdelegate.h" "${ViewPath}/sidebaritemdelegate.cpp"
    FILES_MATCHING PATTERN "${ViewPath}/sidebarwidget.h" "${ViewPath}/sidebarwidget.cpp"
    FILES_MATCHING PATTERN "${ViewPath}/private/sidebarview_p.h"
    )

add_executable(${PROJECT_NAME}
    ${SRC_FILES}
    ${UT_CXX_FILE}
    ${CPP_STUB_SRC}
)

find_package(Dtk REQUIRED Widget REQUIRED)

target_include_directories(${PROJECT_NAME} PRIVATE
    "${PluginPath}")
target_link_libraries(${PROJECT_NAME} PRIVATE
    DFM::base
    DFM::framework
    ${DtkWidget_LIBRARIES}
)

add_test(
  NAME sidebar
  COMMAND $<TARGET_FILE:${PROJECT_NAME}>
)
