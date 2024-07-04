cmake_minimum_required(VERSION 3.10)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

project(test-ddplugin-background)

# UT文件
file(GLOB_RECURSE UT_CXX_FILE
    "${CMAKE_CURRENT_SOURCE_DIR}/*")

set(DESKTOP_PLUGIN_SRC_DIR "${DESKTOP_PLUGIN_DIR}/ddplugin-background")

file(GLOB_RECURSE SRC_FILES
    "${DESKTOP_PLUGIN_SRC_DIR}/*.h"
    "${DESKTOP_PLUGIN_SRC_DIR}/*.cpp"
    "${DESKTOP_PLUGIN_DIR}/desktoputils/*"
)

add_executable(${PROJECT_NAME}
    ${UT_CXX_FILE}
    ${CPP_STUB_SRC}
    ${SRC_FILES}
)
find_package(Qt COMPONENTS Core Widget REQUIRED)
find_package(Dtk COMPONENTS Widget REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(DFrameworkDBus REQUIRED dframeworkdbus )

target_include_directories(${PROJECT_NAME} PRIVATE
    ${DESKTOP_PLUGIN_DIR}
    ${DESKTOP_PLUGIN_SRC_DIR}
    ${DFrameworkDBus_INCLUDE_DIRS}
    ${Qt5Widgets_PRIVATE_INCLUDE_DIRS}
)

target_link_libraries(${PROJECT_NAME}
    DFM::base
    DFM::framework
    ${DFrameworkDBus_LIBRARIES}
)

add_test(
  NAME ddplugin-background
  COMMAND $<TARGET_FILE:${PROJECT_NAME}>
)
