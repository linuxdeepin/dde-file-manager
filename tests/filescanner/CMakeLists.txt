cmake_minimum_required(VERSION 3.10)

project(test-filescanner-demo)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# 生成编译命令数据库（用于 IDE 支持）
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# 查找依赖包
find_package(Qt6 COMPONENTS Core Widgets REQUIRED)
find_package(Dtk6 COMPONENTS Widget REQUIRED)

# 收集源文件
FILE(GLOB_RECURSE FILESCANNER_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
)

# 创建可执行文件
add_executable(${PROJECT_NAME}
    ${FILESCANNER_FILES}
)

# 创建别名（不带 test- 前缀，方便使用）
add_executable(dfm-filescanner-demo ALIAS ${PROJECT_NAME})

# 设置输出目录
set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/tests
)

# 链接 dfm-base 库（使用项目内部目标，无需安装）
target_link_libraries(${PROJECT_NAME} PRIVATE
    dfm6-base
    Qt6::Core
    Qt6::Widgets
    Dtk6::Widget
)

# 设置包含目录
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/src/dfm-base
    ${CMAKE_SOURCE_DIR}/src/dfm-base/base
    ${CMAKE_SOURCE_DIR}/src/dfm-base/utils
    ${CMAKE_SOURCE_DIR}/src/dfm-base/file
    ${CMAKE_SOURCE_DIR}/src/dfm-base/interfaces
)
