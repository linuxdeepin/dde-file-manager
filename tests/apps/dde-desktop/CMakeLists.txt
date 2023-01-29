cmake_minimum_required(VERSION 3.10)

# 定义可执行程序名称
set(BIN_NAME test-dde-desktop)

set(DESKTOP_APP_DIR "${PROJECT_SOURCE_PATH}/apps/dde-desktop")

set(SRCS
    "${DESKTOP_APP_DIR}/desktopdbusinterface.h"
    "${DESKTOP_APP_DIR}/desktopdbusinterface.cpp"
)

#单元测试文件
FILE(GLOB_RECURSE UT_SRC "./*.cpp")

find_package(PkgConfig REQUIRED)
find_package(Qt5 COMPONENTS
    Widgets
    DBus
    REQUIRED)

find_package(Dtk COMPONENTS Widget REQUIRED)

add_executable(${BIN_NAME} ${SRCS} ${UT_SRC} ${CPP_STUB_SRC})

target_include_directories(${BIN_NAME}
    PRIVATE
        ${CMAKE_CURRENT_BINARY_DIR}
        ${DtkWidget_INCLUDE_DIRS}
        ${Qt5DBus_INCLUDE_DIRS}
        ${DESKTOP_APP_DIR}
)

target_link_libraries(
    ${BIN_NAME}
    DFM::base
    DFM::framework
    Qt5::Widgets
    Qt5::DBus
    ${DtkWidget_LIBRARIES}
)
