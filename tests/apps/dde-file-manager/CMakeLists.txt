# 定义需要的cmake版本
cmake_minimum_required(VERSION 3.10)

# 工程名称
project(test-dde-file-manager)

# 依赖包
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

#单元测试文件
FILE(GLOB_RECURSE UT_SRC "./*/*.cpp")

# 源文件
set(SRCS
    main.cpp
)

add_executable(${PROJECT_NAME} ${SRCS})

target_include_directories(${PROJECT_NAME} PUBLIC
    ${CMAKE_SOURCE_DIR}/include
    ${DtkWidget_INCLUDE_DIRS}
    ${DtkGUI_INCLUDE_DIRS}
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    ${Qt_LIBS}
    ${DtkWidget_LIBRARIES}
    ${DtkGUI_LIBRARIES}
    ${GTEST_LIBRARIES}
    ${Qt5Test_LIBRARIES}
    -lpthread
)

add_test(
  NAME dde-file-manager
  COMMAND $<TARGET_FILE:${PROJECT_NAME}>
)

