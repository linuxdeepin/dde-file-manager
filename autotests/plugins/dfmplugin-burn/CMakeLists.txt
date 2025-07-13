cmake_minimum_required(VERSION 3.10)

# dfmplugin-burn Unit Tests
project(test-dfmplugin-burn)

set(PLUGIN_PATH ${PROJECT_SOURCE_PATH}/plugins/common/dfmplugin-burn/)

# UT文件
file(GLOB_RECURSE UT_CXX_FILE
    FILES_MATCHING PATTERN "*.cpp" "*.h")
file(GLOB_RECURSE SRC_FILES
    FILES_MATCHING PATTERN "${PLUGIN_PATH}/*.cpp" "${PLUGIN_PATH}/*.h")

add_executable(${PROJECT_NAME}
    ${SRC_FILES}
    ${UT_CXX_FILE}
    ${CPP_STUB_SRC}
)

find_package(Dtk6 COMPONENTS Widget REQUIRED)

target_include_directories(${PROJECT_NAME} PRIVATE
    "${PLUGIN_PATH}")
target_link_libraries(${PROJECT_NAME} PRIVATE
    DFM6::base
    DFM6::framework
    Dtk6::Widget
)

add_test(
  NAME burn
  COMMAND $<TARGET_FILE:${PROJECT_NAME}>
)
