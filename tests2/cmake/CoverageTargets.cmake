# CoverageTargets.cmake - 覆盖率报告生成模块
# 负责为每个组件生成覆盖率目标，支持HTML和XML格式输出

#[[
函数: dfm_add_coverage_target
用途: 为指定组件创建覆盖率目标
参数: COMPONENT - 组件名称
功能: 
  1. 创建coverage-${COMPONENT}自定义目标
  2. 使用lcov收集覆盖率数据
  3. 过滤只包含src/${COMPONENT}/*的文件
  4. 生成HTML和XML格式报告
]]
function(dfm_add_coverage_target COMPONENT)
    # 检查lcov工具是否可用
    find_program(LCOV_PATH lcov)
    find_program(GENHTML_PATH genhtml)
    
    if(NOT LCOV_PATH OR NOT GENHTML_PATH)
        message(WARNING "lcov 或 genhtml 未找到，跳过 ${COMPONENT} 覆盖率目标")
        return()
    endif()
    
    message(STATUS "为组件 ${COMPONENT} 创建覆盖率目标")
    
    # 设置覆盖率输出目录
    set(COVERAGE_OUTPUT_DIR "${CMAKE_BINARY_DIR}/coverage")
    set(COMPONENT_COVERAGE_DIR "${COVERAGE_OUTPUT_DIR}/${COMPONENT}")
    
    # 创建覆盖率收集目标
    add_custom_target(coverage-${COMPONENT}
        # 创建输出目录
        COMMAND ${CMAKE_COMMAND} -E make_directory ${COMPONENT_COVERAGE_DIR}
        
        # 清理之前的覆盖率数据
        COMMAND ${LCOV_PATH} --directory . --zerocounters
        
        # 运行测试
        COMMAND ${CMAKE_CTEST_COMMAND} -L ${COMPONENT} --output-on-failure
        
        # 收集覆盖率数据
        COMMAND ${LCOV_PATH} --directory . --capture --output-file ${COMPONENT_COVERAGE_DIR}/${COMPONENT}-total.info
        
        # 过滤只包含src/${COMPONENT}/*的文件
        COMMAND ${LCOV_PATH} --extract ${COMPONENT_COVERAGE_DIR}/${COMPONENT}-total.info 
                "*/src/${COMPONENT}/*" 
                --output-file ${COMPONENT_COVERAGE_DIR}/${COMPONENT}-filtered.info
        
        # 生成HTML报告
        COMMAND ${GENHTML_PATH} --output-directory ${COMPONENT_COVERAGE_DIR}/html 
                --title "${COMPONENT} Coverage Report"
                --show-details --legend
                ${COMPONENT_COVERAGE_DIR}/${COMPONENT}-filtered.info
        
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMENT "正在生成 ${COMPONENT} 组件的覆盖率报告..."
        DEPENDS ${COMPONENT}-test-objects
    )
    
    # 创建快速查看目标
    add_custom_target(show-coverage-${COMPONENT}
        COMMAND echo "覆盖率报告位置: ${COMPONENT_COVERAGE_DIR}/html/index.html"
        COMMAND echo "使用浏览器打开: file://${COMPONENT_COVERAGE_DIR}/html/index.html"
        DEPENDS coverage-${COMPONENT}
    )
    
endfunction()

#[[
函数: dfm_setup_coverage_targets
用途: 统一设置覆盖率目标
功能: 
  1. 检查lcov和genhtml工具是否可用
  2. 为所有组件创建覆盖率目标
  3. 创建coverage-all统一目标
  4. 创建clean-coverage清理目标
]]
function(dfm_setup_coverage_targets)
    # 检查覆盖率工具
    find_program(LCOV_PATH lcov)
    find_program(GENHTML_PATH genhtml)
    
    if(NOT LCOV_PATH)
        message(WARNING "lcov 未找到，覆盖率功能将不可用")
        message(STATUS "请安装 lcov: sudo apt-get install lcov")
        return()
    endif()
    
    if(NOT GENHTML_PATH)
        message(WARNING "genhtml 未找到，HTML报告功能将不可用")
        return()
    endif()
    
    message(STATUS "设置统一覆盖率目标...")
    
    # 设置全局覆盖率输出目录
    set(COVERAGE_OUTPUT_DIR "${CMAKE_BINARY_DIR}/coverage")
    
    # 创建覆盖率总目标
    add_custom_target(coverage-all
        COMMAND ${CMAKE_COMMAND} -E make_directory ${COVERAGE_OUTPUT_DIR}
        COMMAND ${LCOV_PATH} --directory . --zerocounters
        COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
        COMMAND ${LCOV_PATH} --directory . --capture --output-file ${COVERAGE_OUTPUT_DIR}/total.info
        COMMAND ${LCOV_PATH} --extract ${COVERAGE_OUTPUT_DIR}/total.info "*/src/*" --output-file ${COVERAGE_OUTPUT_DIR}/filtered.info
        COMMAND ${GENHTML_PATH} --output-directory ${COVERAGE_OUTPUT_DIR}/html --title "DFM Complete Coverage Report" --show-details --legend ${COVERAGE_OUTPUT_DIR}/filtered.info
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMENT "生成完整的覆盖率报告..."
    )
    
    # 创建覆盖率清理目标
    add_custom_target(clean-coverage
        COMMAND ${CMAKE_COMMAND} -E remove_directory ${COVERAGE_OUTPUT_DIR}
        COMMAND ${LCOV_PATH} --directory . --zerocounters
        COMMAND find . -name "*.gcda" -delete
        COMMAND find . -name "*.gcno" -delete
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMENT "清理覆盖率数据..."
    )
    
    # 创建覆盖率摘要目标
    add_custom_target(coverage-summary
        COMMAND ${LCOV_PATH} --directory . --capture --output-file ${COVERAGE_OUTPUT_DIR}/summary.info
        COMMAND ${LCOV_PATH} --extract ${COVERAGE_OUTPUT_DIR}/summary.info "*/src/*" --output-file ${COVERAGE_OUTPUT_DIR}/summary-filtered.info
        COMMAND ${LCOV_PATH} --summary ${COVERAGE_OUTPUT_DIR}/summary-filtered.info
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMENT "显示覆盖率摘要..."
    )
    
    message(STATUS "✅ 覆盖率目标设置完成")
    message(STATUS "   使用 'make coverage-all' 生成完整报告")
    message(STATUS "   使用 'make coverage-summary' 查看摘要")
    message(STATUS "   使用 'make clean-coverage' 清理数据")
endfunction()

#[[
函数: dfm_coverage_summary
用途: 解析并显示覆盖率摘要
功能: 
  1. 解析覆盖率数据
  2. 计算整体覆盖率百分比
  3. 生成覆盖率摘要报告
  4. 支持阈值检查
]]
function(dfm_coverage_summary)
    find_program(LCOV_PATH lcov)
    if(NOT LCOV_PATH)
        message(WARNING "lcov 未找到，无法生成覆盖率摘要")
        return()
    endif()
    
    set(COVERAGE_OUTPUT_DIR "${CMAKE_BINARY_DIR}/coverage")
    
    if(NOT EXISTS "${COVERAGE_OUTPUT_DIR}/filtered.info")
        message(WARNING "未找到覆盖率数据文件，请先运行 make coverage-all")
        return()
    endif()
    
    # 显示覆盖率摘要
    execute_process(
        COMMAND ${LCOV_PATH} --summary ${COVERAGE_OUTPUT_DIR}/filtered.info
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        OUTPUT_VARIABLE COVERAGE_SUMMARY
        ERROR_VARIABLE COVERAGE_ERROR
    )
    
    if(COVERAGE_ERROR)
        message(WARNING "生成覆盖率摘要时出错: ${COVERAGE_ERROR}")
    else()
        message(STATUS "📊 覆盖率摘要:")
        message(STATUS "${COVERAGE_SUMMARY}")
    endif()
endfunction()

# 覆盖率相关的CMake选项
option(ENABLE_COVERAGE "启用覆盖率收集" ON)
set(COVERAGE_THRESHOLD "80" CACHE STRING "覆盖率阈值百分比")
set(COVERAGE_OUTPUT_DIR "coverage" CACHE STRING "覆盖率输出目录")

message(STATUS "CoverageTargets.cmake 已加载 - 覆盖率报告生成模块")