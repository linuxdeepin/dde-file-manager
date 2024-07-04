cmake_minimum_required(VERSION 3.10)

project(test-dfmplugin-workspace)

set(PluginPath ${PROJECT_SOURCE_PATH}/plugins/filemanager/core/dfmplugin-workspace)

# UI files
file(GLOB_RECURSE UT_CXX_FILE
    FILES_MATCHING PATTERN "*.cpp" "*.h")
file(GLOB_RECURSE SRC_FILES
    FILES_MATCHING PATTERN "${PluginPath}/*.cpp" "${PluginPath}/*.h")

add_executable(${PROJECT_NAME}
    ${SRC_FILES}
    ${UT_CXX_FILE}
    ${CPP_STUB_SRC}
)

find_package(Dtk COMPONENTS Widget REQUIRED)

target_include_directories(${PROJECT_NAME} PRIVATE
    "${PluginPath}"
    ${Qt5Widgets_PRIVATE_INCLUDE_DIRS}
    )
target_link_libraries(${PROJECT_NAME} PRIVATE
    DFM::base
    DFM::framework
    ${DtkWidget_LIBRARIES}
)

add_test(
    NAME workspace
    COMMAND $<TARGET_FILE:${PROJECT_NAME}>
)
