# 定义需要的cmake版本
cmake_minimum_required(VERSION 3.10)

# 设置工程名字
project(ut-file-manager)
set(CMAKE_CXX_STANDARD 17)

#if no debug, can't out in code define key '__FUNCTION__' and so on
add_definitions(-DQT_MESSAGELOGCONTEXT)

if(CMAKE_VERSION VERSION_LESS "3.7.0")
    set(CMAKE_INCLUDE_CURRENT_DIR ON)
endif()

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(PROJECT_INCLUDE_PATH "${CMAKE_SOURCE_DIR}/include")
set(PROJECT_SOURCE_PATH "${CMAKE_SOURCE_DIR}/src")
include_directories(${PROJECT_SOURCE_PATH})

# UT依赖
find_package(GTest REQUIRED)
find_package(Qt6 COMPONENTS Test REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})
link_libraries(${GTEST_LIBRARIES} pthread stdc++fs Qt6::Test)

# defines to determind whether to show invoke log at stub function.
add_definitions(-DDEBUG_STUB_INVOKE)

# 打桩工具
set(TEST_UTILS_PATH "${CMAKE_SOURCE_DIR}/3rdparty/testutils")
file(GLOB CPP_STUB_SRC "${TEST_UTILS_PATH}/cpp-stub/*.h"
    "${TEST_UTILS_PATH}/cpp-stub/*.hpp"
    "${TEST_UTILS_PATH}/stub-ext/*.h"
    "${TEST_UTILS_PATH}/stub-ext/*.cpp")
include_directories("${TEST_UTILS_PATH}/cpp-stub"
    "${TEST_UTILS_PATH}/stub-ext")

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set (CMAKE_VERBOSE_MAKEFILE ON)
    add_compile_definitions(ENABLE_TSAN_TOOL)
    message("debug type open sanitize check")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=undefined,address,leak -fno-omit-frame-pointer")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=undefined,address,leak -fno-omit-frame-pointer")
    set(CMAKE_L_FLAGS "${CMAKE_L_FLAGS} -fsanitize=undefined,address,leak -fno-omit-frame-pointer")

    #    set(CMAKE_CXX_FLAGS "-fsanitize=thread")
    #    set(CMAKE_C_FLAGS "-fsanitize=thread")
    #    set(CMAKE_L_FLAGS "-fsanitize=thread")
endif()

# 用于测试覆盖率的编译条件
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall -Wl,--as-needed")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-inline -fno-access-control -O0 -fprofile-arcs -ftest-coverage -lgcov")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DQT_DEBUG")

# add_subdirectory(apps)
add_subdirectory(libs)
add_subdirectory(plugins)
# add_subdirectory(external)
# add_subdirectory(tools)
