# 定义需要的cmake版本
cmake_minimum_required(VERSION 3.10)

# 设置工程名字
project(ut-file-manager)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DQT_DEBUG")

set(PROJECT_SOURCE_PATH "${CMAKE_SOURCE_DIR}/../src")
set(TEST_UTILS_PATH "${CMAKE_SOURCE_DIR}/utils")

# 用于测试覆盖率的编译条件
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-inline -fno-access-control -O0 -fprofile-arcs -ftest-coverage -lgcov")

# UT依赖
find_package(GTest REQUIRED)
find_package(Qt5 COMPONENTS Test REQUIRED)

# 打桩工具
FILE(GLOB CPP_STUB_SRC "${TEST_UTILS_PATH}/cpp-stub/*.h"
    "${TEST_UTILS_PATH}/cpp-stub/*.hpp"
    "${TEST_UTILS_PATH}/stub-ext/*.h"
    "${TEST_UTILS_PATH}/stub-ext/*.cpp")

include_directories("${TEST_UTILS_PATH}/cpp-stub" "${TEST_UTILS_PATH}/stub-ext")
include_directories(${PROJECT_SOURCE_PATH})

add_subdirectory(apps)
add_subdirectory(dde-dock-plugins)
add_subdirectory(dfm-base)
add_subdirectory(dfm-extension)
add_subdirectory(dfm-framework)
add_subdirectory(plugins)
add_subdirectory(services)


