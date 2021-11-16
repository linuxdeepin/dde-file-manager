# 定义需要的cmake版本
cmake_minimum_required(VERSION 3.10)

# 设置工程名字
project(ut-file-manager)

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

# Use Qt modules
set(QtFindModules Core Gui Widgets Concurrent Svg Network Sql X11Extras Xml)
foreach(QtModule ${QtFindModules})
    find_package(Qt5 COMPONENTS ${QtModule} REQUIRED)
    # include qt module private include directors
    include_directories(${Qt5${QtModule}_PRIVATE_INCLUDE_DIRS})
    # can use target_link_libraries(xxx ${QtUseModules})
    list(APPEND QtUseModules "Qt5::${QtModule}")
    message("QtModule found ${QtModule} OK!")
endforeach()

#other not found libs kde from input for Qt
set(KdeFindPackage KF5Codecs)
foreach(KdeConfigModule ${KdeFindPackage})
    find_package(${KdeConfigModule} REQUIRED)
    if(${KdeConfigModule}_FOUND)
        #can use target_link_libraries(xxx ${QtUseModules})
        list(APPEND QtUseModules "${KdeConfigModule}")
        include_directories(${${KdeConfigModule}_INCLUDE_DIRS})
        include_directories(/usr/include/KF5/KCodecs/)
    else()
        message(FATAL_ERROR "NotFound ${KdeConfigModule}")
    endif(${CMakeConfigModule}_FOUND)
endforeach(KdeConfigModule)

# Use PkgConfig modules
# You can setting mini version for "glib-2.0>=2.10 gtk+-2.0" in the list
set(PkgConfigFindModules dtkcore dtkwidget dtkgui gio-unix-2.0 dfm-io)
include(FindPkgConfig)
foreach(PkgConfigModule ${PkgConfigFindModules})
    pkg_check_modules(PkgUseModuleVal REQUIRED ${PkgConfigModule})
    #can use target_link_libraries(xxx ${PkgUseModule})
    list(APPEND PkgUseModules ${PkgUseModuleVal_LDFLAGS})
    include_directories(${PkgUseModuleVal_INCLUDE_DIRS})
    message(PkgUseModuleVal_LINK_LIBRARIES ${PkgUseModuleVal_LINK_LIBRARIES})
    #link_libraries(${PkgUseModuleVal_LIBRARIES})
    link_directories(${PkgUseModuleVal_LINK_LIBRARIES})
endforeach(PkgConfigModule)

set(PROJECT_SOURCE_PATH "${CMAKE_SOURCE_DIR}/../src")
include_directories(${PROJECT_SOURCE_PATH})

# UT依赖
find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})
link_libraries(${GTEST_LIBRARIES} pthread stdc++fs)

# 打桩工具
file(GLOB CPP_STUB_SRC "${TEST_UTILS_PATH}/cpp-stub/*.h"
    "${TEST_UTILS_PATH}/cpp-stub/*.hpp"
    "${TEST_UTILS_PATH}/stub-ext/*.h"
    "${TEST_UTILS_PATH}/stub-ext/*.cpp")
include_directories("${TEST_UTILS_PATH}/cpp-stub"
    "${TEST_UTILS_PATH}/stub-ext")

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set (CMAKE_VERBOSE_MAKEFILE ON)
    message("debug type open sanitize check")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=undefined,address,leak -fno-omit-frame-pointer")
    set(CMAKE_C_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=undefined,address,leak -fno-omit-frame-pointer")
    set(CMAKE_L_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=undefined,address,leak -fno-omit-frame-pointer")
    #    set(CMAKE_CXX_FLAGS "-fsanitize=thread")
    #    set(CMAKE_C_FLAGS "-fsanitize=thread")
    #    set(CMAKE_L_FLAGS "-fsanitize=thread")
endif()

# 用于测试覆盖率的编译条件
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall -Wl,--as-need -fPIE")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-inline -fno-access-control -O0 -fprofile-arcs -ftest-coverage -lgcov")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DQT_DEBUG")

add_subdirectory(apps)
add_subdirectory(dfm-base)
add_subdirectory(dde-dock-plugins)
add_subdirectory(dfm-extension)
add_subdirectory(dfm-framework)
add_subdirectory(plugins)
add_subdirectory(services)


