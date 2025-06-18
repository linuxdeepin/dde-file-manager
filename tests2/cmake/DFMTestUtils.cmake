# DFMTestUtils.cmake - DDE文件管理器测试工具核心模块
# 零侵入测试架构的核心实现
# 
# 主要功能：
# - dfm_create_component_test(): 自动发现源文件并创建测试目标
# - dfm_auto_link_dependencies(): 智能依赖链接
# - dfm_discover_test_files(): 测试文件发现和注册

#[[
函数: dfm_create_component_test
用途: 为指定组件创建测试目标，实现零侵入的自动文件发现
参数: COMPONENT_NAME - 组件名称（如：dfm-framework, dfm-base等）
功能: 
  1. 自动发现${DFM_SOURCE_DIR}/src/${COMPONENT_NAME}/*.cpp文件
  2. 智能过滤排除test、main.cpp、build目录下的文件
  3. 创建${COMPONENT_NAME}-test-objects对象库
  4. 应用覆盖率编译选项
  5. 自动配置include路径
  6. 调用依赖链接函数
]]
function(dfm_create_component_test COMPONENT_NAME)
    message(STATUS "正在为组件 ${COMPONENT_NAME} 创建测试目标...")
    
    # 自动发现组件源文件 - 完全不修改src目录
    set(SRC_DIR "${DFM_SOURCE_DIR}/src/${COMPONENT_NAME}")
    
    # 检查源目录是否存在
    if(NOT EXISTS ${SRC_DIR})
        message(WARNING "组件源目录不存在: ${SRC_DIR}")
        return()
    endif()
    
    # 递归搜索所有cpp文件
    file(GLOB_RECURSE COMPONENT_SOURCES 
        CONFIGURE_DEPENDS
        "${SRC_DIR}/*.cpp"
    )
    
    # 递归搜索所有头文件 - MOC需要
    file(GLOB_RECURSE COMPONENT_HEADERS 
        CONFIGURE_DEPENDS
        "${SRC_DIR}/*.h"
        "${DFM_SOURCE_DIR}/include/${COMPONENT_NAME}/*.h"
    )
    
    # 智能过滤不需要的文件 - 使用手动遍历方式
    set(FILTERED_SOURCES "")
    foreach(SOURCE_FILE ${COMPONENT_SOURCES})
        get_filename_component(FILENAME ${SOURCE_FILE} NAME)
        get_filename_component(FILEPATH ${SOURCE_FILE} DIRECTORY)
        
        # 检查是否是测试文件 - 只检查文件名
        string(FIND ${FILENAME} "test" TEST_POS)
        string(FIND ${FILENAME} "Test" TEST_POS2)
        string(FIND ${FILENAME} "TEST" TEST_POS3)
        
        # 检查是否是main文件
        string(COMPARE EQUAL ${FILENAME} "main.cpp" IS_MAIN)
        string(COMPARE EQUAL ${FILENAME} "Main.cpp" IS_MAIN2)
        
        # 检查是否在build目录
        string(FIND ${FILEPATH} "/build" BUILD_POS)
        string(FIND ${FILEPATH} "/CMakeFiles" CMAKE_POS)
        
        # 如果不是测试文件、main文件或构建目录中的文件，则保留
        if(TEST_POS EQUAL -1 AND TEST_POS2 EQUAL -1 AND TEST_POS3 EQUAL -1 AND 
           NOT IS_MAIN AND NOT IS_MAIN2 AND BUILD_POS EQUAL -1 AND CMAKE_POS EQUAL -1)
            list(APPEND FILTERED_SOURCES ${SOURCE_FILE})
        endif()
    endforeach()
    
    set(COMPONENT_SOURCES ${FILTERED_SOURCES})
    
    # 检查是否找到源文件
    list(LENGTH COMPONENT_SOURCES SOURCE_COUNT)
    if(SOURCE_COUNT EQUAL 0)
        message(WARNING "组件 ${COMPONENT_NAME} 未发现可用的源文件")
        return()
    endif()
    
    message(STATUS "  发现 ${SOURCE_COUNT} 个源文件:")
    foreach(SOURCE_FILE ${COMPONENT_SOURCES})
        file(RELATIVE_PATH REL_PATH ${DFM_SOURCE_DIR} ${SOURCE_FILE})
        message(STATUS "    ${REL_PATH}")
    endforeach()
    
    # 创建测试专用对象库 - 包含源文件和头文件
    set(TEST_OBJ_NAME "${COMPONENT_NAME}-test-objects")
    
    # 查找需要MOC处理的头文件
    set(MOC_HEADERS "")
    foreach(HEADER_FILE ${COMPONENT_HEADERS})
        file(READ ${HEADER_FILE} HEADER_CONTENT)
        string(FIND "${HEADER_CONTENT}" "Q_OBJECT" Q_OBJECT_POS)
        if(NOT Q_OBJECT_POS EQUAL -1)
            list(APPEND MOC_HEADERS ${HEADER_FILE})
            message(STATUS "    需要MOC处理: ${HEADER_FILE}")
        endif()
    endforeach()
    
    # 手动生成MOC文件
    set(MOC_SOURCES "")
    foreach(MOC_HEADER ${MOC_HEADERS})
        get_filename_component(HEADER_NAME ${MOC_HEADER} NAME_WE)
        set(MOC_OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/moc_${HEADER_NAME}.cpp")
        
        # 使用Qt6的moc工具生成MOC文件
        add_custom_command(
            OUTPUT ${MOC_OUTPUT}
            COMMAND Qt6::moc 
                -I${DFM_SOURCE_DIR}/include 
                -I${DFM_SOURCE_DIR}/src
                -I${DFM_SOURCE_DIR}/src/${COMPONENT_NAME}
                ${MOC_HEADER} 
                -o ${MOC_OUTPUT}
            DEPENDS ${MOC_HEADER}
            COMMENT "生成MOC文件: ${MOC_OUTPUT}"
        )
        
        list(APPEND MOC_SOURCES ${MOC_OUTPUT})
    endforeach()
    
    add_library(${TEST_OBJ_NAME} OBJECT 
        ${COMPONENT_SOURCES}
        ${MOC_SOURCES}  # 包含手动生成的MOC文件
    )
    
    # 设置目标属性
    set_target_properties(${TEST_OBJ_NAME} PROPERTIES
        AUTOMOC OFF  # 关闭自动MOC，因为我们手动处理
        AUTOUIC ON
        AUTORCC ON
    )
    
    # 应用覆盖率和测试专用编译选项
    target_compile_options(${TEST_OBJ_NAME} PRIVATE
        -fprofile-arcs          # gcov覆盖率数据生成
        -ftest-coverage         # 测试覆盖率支持
        -fno-access-control     # 访问私有成员（用于白盒测试）
        -O0                     # 禁用优化以获得准确的覆盖率
        -g                      # 调试信息
    )
    
    # 设置覆盖率链接选项
    target_link_options(${TEST_OBJ_NAME} PRIVATE
        -fprofile-arcs
        -ftest-coverage
    )
    
    # 自动配置include路径
    target_include_directories(${TEST_OBJ_NAME} PRIVATE
        ${DFM_SOURCE_DIR}/src              # 源代码根目录
        ${DFM_SOURCE_DIR}/include          # 公共头文件目录
        ${DFM_SOURCE_DIR}/src/${COMPONENT_NAME}  # 组件特定目录
    )
    
    # 自动推导并链接依赖
    dfm_auto_link_dependencies(${TEST_OBJ_NAME} ${COMPONENT_NAME})
    
    # 自动发现并创建测试可执行文件
    dfm_discover_test_files(${COMPONENT_NAME} ${TEST_OBJ_NAME})
    
    message(STATUS "✅ 组件 ${COMPONENT_NAME} 测试目标创建完成")
endfunction()

#[[
函数: dfm_auto_link_dependencies
用途: 为指定目标自动配置依赖关系
参数: TARGET_NAME - 目标名称
      COMPONENT_NAME - 组件名称
功能: 根据组件名自动推导并链接所需的依赖库
]]
function(dfm_auto_link_dependencies TARGET_NAME COMPONENT_NAME)
    message(STATUS "  配置 ${COMPONENT_NAME} 组件依赖...")
    
    # 通用依赖 - 所有组件都需要
    target_link_libraries(${TARGET_NAME} PRIVATE
        Qt6::Core
    )
    
    # 根据组件名自动推导特定依赖关系
    if(${COMPONENT_NAME} STREQUAL "dfm-framework")
        # dfm-framework 组件依赖
        target_link_libraries(${TARGET_NAME} PRIVATE 
            Qt6::Core 
            Qt6::Concurrent
        )
        
        # 查找并链接Dtk6::Core - 使用更强制的方式
        find_package(Dtk6 COMPONENTS Core REQUIRED)
        if(Dtk6_FOUND)
            target_link_libraries(${TARGET_NAME} PRIVATE Dtk6::Core)
            message(STATUS "    ✅ 链接 Dtk6::Core")
        else()
            message(FATAL_ERROR "❌ 未找到 Dtk6::Core，dfm-framework组件需要此依赖")
        endif()
        
    elseif(${COMPONENT_NAME} STREQUAL "dfm-base")
        # dfm-base 组件依赖
        target_link_libraries(${TARGET_NAME} PRIVATE 
            Qt6::Core 
            Qt6::Widgets 
            Qt6::DBus
        )
        
        # 查找并链接Dtk6::Core
        find_package(Dtk6 COMPONENTS Core REQUIRED)
        if(Dtk6_FOUND)
            target_link_libraries(${TARGET_NAME} PRIVATE Dtk6::Core)
            message(STATUS "    ✅ 链接 Dtk6::Core")
        else()
            message(FATAL_ERROR "❌ 未找到 Dtk6::Core，dfm-base组件需要此依赖")
        endif()
        
    elseif(${COMPONENT_NAME} STREQUAL "dfm-extension")
        # dfm-extension 组件依赖
        target_link_libraries(${TARGET_NAME} PRIVATE 
            Qt6::Core
        )
        
    else()
        # 未知组件，使用默认依赖
        message(STATUS "    使用默认依赖配置（仅Qt6::Core）")
    endif()
    
    message(STATUS "  ✅ 依赖配置完成")
endfunction()

#[[
函数: dfm_discover_test_files
用途: 发现当前目录下的测试文件并创建可执行目标
参数: COMPONENT_NAME - 组件名称
      TEST_OBJ_NAME - 测试对象库名称
功能: 
  1. 发现当前目录下的test_*.cpp文件
  2. 为每个测试文件创建可执行目标
  3. 链接到对应的test-objects库
  4. 链接测试框架
  5. 注册到CTest
]]
function(dfm_discover_test_files COMPONENT_NAME TEST_OBJ_NAME)
    message(STATUS "  发现 ${COMPONENT_NAME} 组件的测试文件...")
    
    # 发现当前目录下的测试文件
    file(GLOB_RECURSE TEST_SOURCES 
        RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
        "test_*.cpp"
    )
    
    list(LENGTH TEST_SOURCES TEST_COUNT)
    if(TEST_COUNT EQUAL 0)
        message(STATUS "    未发现测试文件 (test_*.cpp)")
        return()
    endif()
    
    message(STATUS "    发现 ${TEST_COUNT} 个测试文件:")
    
    # 为每个测试文件创建可执行目标
    foreach(TEST_SOURCE ${TEST_SOURCES})
        # 获取测试名称（去掉扩展名）
        get_filename_component(TEST_NAME ${TEST_SOURCE} NAME_WE)
        set(FULL_TEST_NAME "${COMPONENT_NAME}-${TEST_NAME}")
        
        message(STATUS "      ${TEST_SOURCE} -> ${FULL_TEST_NAME}")
        
        # 创建测试可执行文件
        add_executable(${FULL_TEST_NAME}
            ${TEST_SOURCE}
            $<TARGET_OBJECTS:${TEST_OBJ_NAME}>  # 包含带覆盖率的源码对象
        )
        
        # 链接测试框架和覆盖率库
        target_link_libraries(${FULL_TEST_NAME} PRIVATE
            Qt6::Test           # Qt6测试框架
            GTest::GTest        # Google Test框架
            GTest::Main         # Google Test主函数
            gcov                # 覆盖率库
        )
        
        # 继承对象库的依赖 - 这很重要！
        target_link_libraries(${FULL_TEST_NAME} PRIVATE
            $<TARGET_PROPERTY:${TEST_OBJ_NAME},LINK_LIBRARIES>
        )
        
        # 手动添加组件特定的依赖
        if(${COMPONENT_NAME} STREQUAL "dfm-framework")
            target_link_libraries(${FULL_TEST_NAME} PRIVATE 
                Qt6::Core 
                Qt6::Concurrent
                Dtk6::Core
            )
        elseif(${COMPONENT_NAME} STREQUAL "dfm-base")
            target_link_libraries(${FULL_TEST_NAME} PRIVATE 
                Qt6::Core 
                Qt6::Widgets 
                Qt6::DBus
                Dtk6::Core
            )
        endif()
        
        # 继承对象库的include目录和编译选项
        target_include_directories(${FULL_TEST_NAME} PRIVATE
            ${DFM_SOURCE_DIR}/src
            ${DFM_SOURCE_DIR}/include
            ${DFM_SOURCE_DIR}/src/${COMPONENT_NAME}
            ${CMAKE_CURRENT_SOURCE_DIR}  # 测试文件所在目录
            ${CMAKE_CURRENT_SOURCE_DIR}/../../framework  # 测试框架头文件
        )
        
        # 设置测试运行时属性
        set_target_properties(${FULL_TEST_NAME} PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/tests/${COMPONENT_NAME}"
        )
        
        # 注册到CTest
        add_test(NAME ${FULL_TEST_NAME} 
                 COMMAND ${FULL_TEST_NAME}
                 WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
        
        # 设置测试标签，便于分组运行
        set_tests_properties(${FULL_TEST_NAME} PROPERTIES
            LABELS "${COMPONENT_NAME}"
            TIMEOUT 300  # 5分钟超时
        )
    endforeach()
    
    message(STATUS "  ✅ 测试文件发现完成")
endfunction()

#[[
函数: dfm_print_component_summary
用途: 打印组件测试配置摘要
参数: COMPONENT_NAME - 组件名称
]]
function(dfm_print_component_summary COMPONENT_NAME)
    message(STATUS "")
    message(STATUS "========================================")
    message(STATUS "组件测试配置摘要: ${COMPONENT_NAME}")
    message(STATUS "========================================")
    message(STATUS "源目录: ${DFM_SOURCE_DIR}/src/${COMPONENT_NAME}")
    message(STATUS "测试目录: ${CMAKE_CURRENT_SOURCE_DIR}")
    message(STATUS "对象库: ${COMPONENT_NAME}-test-objects")
    message(STATUS "========================================")
    message(STATUS "")
endfunction()

# 工具函数：验证组件有效性
function(dfm_validate_component_exists COMPONENT_NAME RESULT_VAR)
    set(SRC_DIR "${DFM_SOURCE_DIR}/src/${COMPONENT_NAME}")
    if(EXISTS ${SRC_DIR})
        set(${RESULT_VAR} TRUE PARENT_SCOPE)
    else()
        set(${RESULT_VAR} FALSE PARENT_SCOPE)
    endif()
endfunction()

message(STATUS "DFMTestUtils.cmake 已加载 - 零侵入测试架构核心模块")