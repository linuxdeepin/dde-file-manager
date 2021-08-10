# 定义需要的cmake版本
cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall -Wl,--as-need -fPIE")

# 定义可执行程序名称
set(BIN_NAME test-file-manager)

# 依赖包
find_package(PkgConfig REQUIRED)
find_package(DtkWidget REQUIRED)
find_package(DtkGui REQUIRED)
find_package(DtkCMake REQUIRED)
find_package(Qt5 COMPONENTS
    Core
    Gui
    Widgets
    Concurrent
    DBus
REQUIRED)

set(Qt_LIBS
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
    Qt5::DBus
)

set(PROJECT_SRC_PATH "${PROJECT_SOURCE_PATH}/apps/dde-file-manager")
include_directories(${PROJECT_SRC_PATH})

#项目工程文件
FILE(GLOB_RECURSE PRO_SRC "${PROJECT_SRC_PATH}/*/*.h" "${PROJECT_SRC_PATH}/*/*.cpp")

#单元测试文件
FILE(GLOB_RECURSE UT_SRC "./*/*.cpp")

# 源文件
set(SRCS
    main.cpp
    ut_demo.cpp
    ../../../src/apps/dde-file-manager/uttestclass.cpp
    ${GLOBAL_SRC}
    ${PRO_SRC}
    ${UT_SRC}
    ${CPP_STUB_SRC}
)

add_executable(${BIN_NAME} ${SRCS})
target_include_directories(${BIN_NAME} PUBLIC
    ${CMAKE_SOURCE_DIR}/include
    ${DtkWidget_INCLUDE_DIRS}
    ${DtkGUI_INCLUDE_DIRS}
)

target_link_libraries(${BIN_NAME} PRIVATE
    ${Qt_LIBS}
    ${DtkWidget_LIBRARIES}
    ${DtkGUI_LIBRARIES}
    ${GTEST_LIBRARIES}
    ${Qt5Test_LIBRARIES}
    -lpthread
)


