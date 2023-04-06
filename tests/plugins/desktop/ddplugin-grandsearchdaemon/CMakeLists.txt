cmake_minimum_required(VERSION 3.10)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

project(test-ddplugin-grandsearchdaemon)

add_definitions(-DGRANDSEARCHDAEMON_LIB_DIR="${CMAKE_INSTALL_FULL_LIBDIR}/dde-grand-search-daemon")

# UT文件
file(GLOB_RECURSE UT_CXX_FILE
    "${CMAKE_CURRENT_SOURCE_DIR}/*")

set(DESKTOP_PLUGIN_SRC_DIR "${DESKTOP_PLUGIN_DIR}/ddplugin-grandsearchdaemon")

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

find_package(Dtk COMPONENTS Widget REQUIRED)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${DESKTOP_PLUGIN_DIR}
    ${DESKTOP_PLUGIN_SRC_DIR}
)

target_link_libraries(${PROJECT_NAME}
    DFM::base
    DFM::framework
)

add_test(
  NAME ddplugin-grandsearchdaemon
  COMMAND $<TARGET_FILE:${PROJECT_NAME}>
)
