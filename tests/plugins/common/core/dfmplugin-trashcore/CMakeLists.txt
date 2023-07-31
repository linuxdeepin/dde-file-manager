cmake_minimum_required(VERSION 3.10)

project(test-dfmplugin-trashcore)

set(PluginPath ${PROJECT_SOURCE_PATH}/plugins/common/core/dfmplugin-trashcore/)

# UT文件
file(GLOB_RECURSE UT_CXX_FILE
    FILES_MATCHING PATTERN "*.cpp" "*.h")
file(GLOB_RECURSE SRC_FILES
    FILES_MATCHING PATTERN "${PluginPath}/*.cpp" "${PluginPath}/*.h")

find_package(Dtk COMPONENTS Widget REQUIRED)
find_package(Qt5 COMPONENTS Core REQUIRED)
find_package(Qt5 COMPONENTS DBus REQUIRED)
find_package(PkgConfig REQUIRED)

pkg_search_module(zlib REQUIRED zlib IMPORTED_TARGET)

add_executable(${PROJECT_NAME}
    ${SRC_FILES}
    ${UT_CXX_FILE}
    ${CPP_STUB_SRC}
)

target_include_directories(${PROJECT_NAME} PRIVATE
    "${PluginPath}")
target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt5::Core
    Qt5::DBus
    DFM::base
    DFM::framework
    PkgConfig::zlib
    ${DtkWidget_LIBRARIES}
)

add_test(
  NAME trashcore
  COMMAND $<TARGET_FILE:${PROJECT_NAME}>
)
