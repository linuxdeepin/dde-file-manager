# TestDiscovery.cmake - 自动测试发现模块
# 负责自动发现units目录下的组件并调用相应的测试创建函数

#[[
函数: dfm_discover_tests
用途: 主函数，自动发现测试组件
功能: 
  1. 扫描units/目录下的所有CMakeLists.txt文件
  2. 提取组件目录名
  3. 使用add_subdirectory()添加组件测试目录
  4. 输出发现的组件列表到控制台
]]
function(dfm_discover_tests)
    message(STATUS "========================================")
    message(STATUS "开始自动发现测试组件...")
    message(STATUS "========================================")
    
    # 扫描units目录下的所有CMakeLists.txt文件
    set(UNITS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/units")
    
    if(NOT EXISTS ${UNITS_DIR})
        message(WARNING "测试units目录不存在: ${UNITS_DIR}")
        return()
    endif()
    
    file(GLOB_RECURSE COMPONENT_CMAKE_FILES 
        RELATIVE "${UNITS_DIR}"
        "${UNITS_DIR}/*/CMakeLists.txt"
    )
    
    list(LENGTH COMPONENT_CMAKE_FILES COMPONENT_COUNT)
    if(COMPONENT_COUNT EQUAL 0)
        message(STATUS "未发现任何组件测试目录")
        return()
    endif()
    
    message(STATUS "发现 ${COMPONENT_COUNT} 个组件:")
    
    set(DISCOVERED_COMPONENTS "")
    
    # 处理每个找到的组件
    foreach(CMAKE_FILE ${COMPONENT_CMAKE_FILES})
        # 提取组件目录名
        get_filename_component(COMPONENT_DIR ${CMAKE_FILE} DIRECTORY)
        
        # 验证组件有效性
        dfm_validate_component(${COMPONENT_DIR} IS_VALID)
        
        if(IS_VALID)
            message(STATUS "  ✅ ${COMPONENT_DIR}")
            
            # 添加到构建系统
            add_subdirectory("units/${COMPONENT_DIR}")
            
            # 记录已发现的组件
            list(APPEND DISCOVERED_COMPONENTS ${COMPONENT_DIR})
        else()
            message(STATUS "  ❌ ${COMPONENT_DIR} (无效或无源文件)")
        endif()
    endforeach()
    
    # 打印发现摘要
    dfm_print_discovery_summary("${DISCOVERED_COMPONENTS}")
    
    message(STATUS "========================================")
    message(STATUS "测试组件发现完成")
    message(STATUS "========================================")
endfunction()

#[[
函数: dfm_validate_component
用途: 验证组件有效性
参数: COMPONENT_NAME - 组件名称
      RESULT_VAR - 结果变量名
功能: 
  1. 检查src/${COMPONENT_NAME}目录是否存在
  2. 检查是否有.cpp源文件
  3. 返回验证结果
]]
function(dfm_validate_component COMPONENT_NAME RESULT_VAR)
    set(SRC_DIR "${DFM_SOURCE_DIR}/src/${COMPONENT_NAME}")
    
    # 调试输出
    message(STATUS "  🔍 验证组件: ${COMPONENT_NAME}")
    message(STATUS "      DFM_SOURCE_DIR: ${DFM_SOURCE_DIR}")
    message(STATUS "      SRC_DIR: ${SRC_DIR}")
    
    # 检查源目录是否存在
    if(NOT EXISTS ${SRC_DIR})
        message(STATUS "      ❌ 源目录不存在")
        set(${RESULT_VAR} FALSE PARENT_SCOPE)
        return()
    endif()
    
    message(STATUS "      ✅ 源目录存在")
    
    # 检查是否有cpp源文件
    file(GLOB_RECURSE CPP_FILES "${SRC_DIR}/*.cpp")
    
    list(LENGTH CPP_FILES TOTAL_COUNT)
    message(STATUS "      📁 总共找到 ${TOTAL_COUNT} 个cpp文件")
    
    # 显示前5个文件作为调试
    set(COUNT 0)
    foreach(FILE ${CPP_FILES})
        if(COUNT LESS 5)
            message(STATUS "        - ${FILE}")
            math(EXPR COUNT "${COUNT} + 1")
        endif()
    endforeach()
    
    # 使用更安全的过滤方法 - 手动遍历而不是使用list(FILTER)
    set(FILTERED_FILES "")
    foreach(FILE ${CPP_FILES})
        get_filename_component(FILENAME ${FILE} NAME)
        get_filename_component(FILEPATH ${FILE} DIRECTORY)
        
        # 检查是否是测试文件 - 只检查文件名，不检查路径
        string(FIND ${FILENAME} "test" TEST_POS)
        string(FIND ${FILENAME} "Test" TEST_POS2)
        string(FIND ${FILENAME} "TEST" TEST_POS3)
        
        # 检查是否是main文件
        string(COMPARE EQUAL ${FILENAME} "main.cpp" IS_MAIN)
        string(COMPARE EQUAL ${FILENAME} "Main.cpp" IS_MAIN2)
        
        # 如果不是测试文件也不是main文件，则保留
        if(TEST_POS EQUAL -1 AND TEST_POS2 EQUAL -1 AND TEST_POS3 EQUAL -1 AND 
           NOT IS_MAIN AND NOT IS_MAIN2)
            list(APPEND FILTERED_FILES ${FILE})
            message(STATUS "        ✅ 保留: ${FILENAME}")
        else()
            message(STATUS "        ❌ 过滤: ${FILENAME} (测试或main文件)")
        endif()
    endforeach()
    
    list(LENGTH FILTERED_FILES SOURCE_COUNT)
    message(STATUS "      📊 过滤后剩余 ${SOURCE_COUNT} 个源文件")
    
    if(SOURCE_COUNT GREATER 0)
        message(STATUS "      ✅ 组件有效")
        set(${RESULT_VAR} TRUE PARENT_SCOPE)
    else()
        message(STATUS "      ❌ 过滤后无源文件")
        set(${RESULT_VAR} FALSE PARENT_SCOPE)
    endif()
endfunction()

#[[
函数: dfm_print_discovery_summary
用途: 打印发现结果摘要
参数: COMPONENTS_LIST - 发现的组件列表
功能: 
  1. 统计发现的组件数量
  2. 统计测试文件数量
  3. 打印发现结果摘要
]]
function(dfm_print_discovery_summary COMPONENTS_LIST)
    list(LENGTH COMPONENTS_LIST COMPONENT_COUNT)
    
    message(STATUS "")
    message(STATUS "📊 发现摘要:")
    message(STATUS "   组件数量: ${COMPONENT_COUNT}")
    
    if(COMPONENT_COUNT GREATER 0)
        message(STATUS "   发现的组件:")
        foreach(COMPONENT ${COMPONENTS_LIST})
            # 统计测试文件数量
            file(GLOB TEST_FILES "${CMAKE_CURRENT_SOURCE_DIR}/units/${COMPONENT}/test_*.cpp")
            list(LENGTH TEST_FILES TEST_COUNT)
            message(STATUS "     - ${COMPONENT} (${TEST_COUNT} 个测试文件)")
        endforeach()
    endif()
    
    message(STATUS "")
endfunction()

message(STATUS "TestDiscovery.cmake 已加载 - 自动测试发现模块")