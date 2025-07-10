# autotests/CMakeLists.txt - DDE文件管理器新测试架构根配置
# 基于零侵入自动文件发现的现代化测试基础设施

cmake_minimum_required(VERSION 3.16)

# 设置项目信息
project(dfm-autotests
    VERSION 1.0.0
    DESCRIPTION "DDE File Manager Test Infrastructure"
    LANGUAGES CXX
)

# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 启用导出编译命令（用于IDE支持）
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# 设置源代码根目录
set(DFM_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/..")

message(STATUS "========================================")
message(STATUS "DDE文件管理器测试系统 v${PROJECT_VERSION}")
message(STATUS "========================================")
message(STATUS "源代码目录: ${DFM_SOURCE_DIR}")
message(STATUS "测试目录: ${CMAKE_CURRENT_SOURCE_DIR}")
message(STATUS "构建目录: ${CMAKE_BINARY_DIR}")
message(STATUS "========================================")

# Qt6配置
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# 查找必要的依赖
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS 
    Core 
    Test 
    Widgets
    Concurrent
    DBus
)

# 查找Google Test
find_package(GTest REQUIRED)
if(GTest_FOUND)
    message(STATUS "✅ Google Test 找到: ${GTEST_VERSION}")
else()
    message(FATAL_ERROR "❌ Google Test 未找到，请安装 libgtest-dev")
endif()

# 查找PkgConfig（用于Dtk依赖）
find_package(PkgConfig REQUIRED)
if(PkgConfig_FOUND)
    message(STATUS "✅ PkgConfig 找到")
else()
    message(FATAL_ERROR "❌ PkgConfig 未找到")
endif()

# 查找覆盖率工具
find_program(LCOV_PATH lcov)
find_program(GENHTML_PATH genhtml)
if(LCOV_PATH AND GENHTML_PATH)
    message(STATUS "✅ 覆盖率工具找到: lcov, genhtml")
else()
    message(WARNING "⚠️  覆盖率工具未完全找到，某些功能可能不可用")
    message(STATUS "   安装命令: sudo apt-get install lcov")
endif()

# 添加编译定义
add_definitions(-DQT_MESSAGELOGCONTEXT)

# 设置编译选项
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    # Debug模式：启用所有调试和覆盖率选项
    add_compile_options(
        -g                          # 调试信息
        -O0                         # 禁用优化
        -fprofile-arcs              # gcov覆盖率数据生成
        -ftest-coverage             # 测试覆盖率支持
        -fno-access-control         # 访问私有成员（用于白盒测试）
        --coverage                  # 覆盖率支持
    )
    
    # 链接器选项
    add_link_options(
        --coverage                  # 覆盖率支持
    )
    
    message(STATUS "📊 Debug模式：启用覆盖率和调试选项")
    
    # 可选：启用Address Sanitizer（需要时取消注释）
    # add_compile_options(-fsanitize=address,undefined,leak)
    # add_link_options(-fsanitize=address,undefined,leak)
    
else()
    # Release模式：优化但保留调试信息
    add_compile_options(-O2 -g)
    message(STATUS "🚀 Release模式：启用优化")
endif()

# 包含测试工具模块
include(cmake/DFMTestUtils.cmake)
include(cmake/TestDiscovery.cmake)
include(cmake/CoverageTargets.cmake)

# 设置CTest
enable_testing()

# 调用测试发现函数
dfm_discover_tests()

# 设置覆盖率目标
if(LCOV_PATH AND GENHTML_PATH)
    dfm_setup_coverage_targets()
endif()

# 添加便利目标
add_custom_target(build-tests
    COMMENT "构建所有测试..."
)

add_custom_target(run-tests
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
    COMMENT "运行所有测试..."
)

add_custom_target(test-help
    COMMAND echo "可用的测试目标:"
    COMMAND echo "  make build-tests     - 构建所有测试"
    COMMAND echo "  make run-tests       - 运行所有测试"
    COMMAND echo "  make coverage-all    - 生成完整覆盖率报告"
    COMMAND echo "  make coverage-summary - 显示覆盖率摘要"
    COMMAND echo "  make clean-coverage  - 清理覆盖率数据"
    COMMAND echo ""
    COMMAND echo "组件特定目标:"
    COMMAND echo "  make coverage-dfm-framework - dfm-framework覆盖率"
    COMMAND echo "  make show-coverage-dfm-framework - 显示报告位置"
    COMMENT "显示帮助信息"
)

message(STATUS "")
message(STATUS "🎯 测试系统配置完成！")
message(STATUS "   运行 'make test-help' 查看可用命令")
message(STATUS "   运行 'make run-tests' 执行所有测试")
message(STATUS "   运行 'make coverage-all' 生成覆盖率报告")
message(STATUS "========================================")
message(STATUS "")
