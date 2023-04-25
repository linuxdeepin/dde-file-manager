cmake_minimum_required(VERSION 3.10)

project(test-dfmplugin-utils)

set(EXTENSIONS_LIB_DIR ${DFM_PLUGIN_DIR}/extensions)
set(PluginPath ${PROJECT_SOURCE_PATH}/plugins/common/dfmplugin-utils/)
add_definitions(-DEXTENSIONS_PATH=\"${EXTENSIONS_LIB_DIR}\")
# UT文件
file(GLOB_RECURSE UT_CXX_FILE
    FILES_MATCHING PATTERN "*.cpp" "*.h")
file(GLOB_RECURSE SRC_FILES
    FILES_MATCHING PATTERN "${PluginPath}/*.cpp" "${PluginPath}/*.h"
    )

find_package(Dtk COMPONENTS Widget REQUIRED)
find_package(Qt5 COMPONENTS DBus REQUIRED)
find_package(Qt5 COMPONENTS Svg REQUIRED)

add_executable(${PROJECT_NAME}
    ${SRC_FILES}
    ${UT_CXX_FILE}
    ${CPP_STUB_SRC}
)

target_include_directories(${PROJECT_NAME} PRIVATE
    "${PluginPath}"
    ${DtkWidget_INCLUDE_DIRS}
    ${Qt5Svg_INCLUDE_DIRS}
)
target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt5::DBus
    DFM::base
    DFM::framework
    DFM::base
    DFM::extension
    Qt5::Svg
    ${DtkWidget_LIBRARIES}
)

add_test(
  NAME utils
  COMMAND $<TARGET_FILE:${PROJECT_NAME}>
)
