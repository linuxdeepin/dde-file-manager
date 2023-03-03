cmake_minimum_required(VERSION 3.10)

project(test-dfmplugin-bookmark)

set(PluginPath ${PROJECT_SOURCE_PATH}/plugins/common/core/dfmplugin-bookmark/)
set(ControllerPath ${PluginPath}/controller/)

include_directories(${ControllerPath})
# UT文件
file(GLOB_RECURSE UT_CXX_FILE
    FILES_MATCHING PATTERN "*.cpp" "*.h")
file(GLOB_RECURSE SRC_FILES
    FILES_MATCHING PATTERN "${PluginPath}/*.cpp" "${PluginPath}/*.h"
    FILES_MATCHING PATTERN "${ControllerPath}/*.cpp" "${ControllerPath}/*.h"
    )


add_executable(${PROJECT_NAME}
    ${SRC_FILES}
    ${UT_CXX_FILE}
    ${CPP_STUB_SRC}
)

find_package(Dtk COMPONENTS Widget REQUIRED)

target_include_directories(${PROJECT_NAME} PRIVATE
    "${PluginPath}")
target_include_directories(${PROJECT_NAME} PRIVATE
    "${ControllerPath}")
target_link_libraries(${PROJECT_NAME} PRIVATE
    DFM::base
    DFM::framework
    ${DtkWidget_LIBRARIES}
)

add_test(
  NAME bookmark
  COMMAND $<TARGET_FILE:${PROJECT_NAME}>
)
