cmake_minimum_required(VERSION 3.10)

project(test-video-preview)

set(PluginPath ${PROJECT_SOURCE_PATH}/plugins/common/dfmplugin-preview/pluginpreviews/video-preview/)

add_definitions(-DPLUGINDIR=\"${DFM_PLUGIN_PREVIEW_DIR}\")

set(VERSION 1.8.2)
add_definitions(-DVERSION=\"${VERSION}\")

# UT文件
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

find_package(PkgConfig REQUIRED)
pkg_search_module(libdmr REQUIRED libdmr)

target_include_directories(${PROJECT_NAME} PRIVATE
    "${PluginPath}"
    "${PROJECT_SOURCE_PATH}/plugins/common/dfmplugin-preview/pluginpreviews"
    ${libdmr_INCLUDE_DIRS}
)
target_link_libraries(${PROJECT_NAME} PRIVATE
    DFM::base
    DFM::framework
    ${DtkWidget_LIBRARIES}
    ${libdmr_LIBRARIES}
)

add_test(
  NAME videopreview
  COMMAND $<TARGET_FILE:${PROJECT_NAME}>
)
