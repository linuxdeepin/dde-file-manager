# autotests/units/dfm-extension/CMakeLists.txt - dfm-extension组件测试配置
# 使用零侵入架构，仅需一行配置即可完成整个组件的测试设置

message(STATUS "配置dfm-extension组件测试...")

# 核心：仅需一行配置，自动发现源文件并创建测试目标
dfm_create_component_test(dfm-extension)

# 为dfm-extension测试添加stub实现
# 这些全局函数是接口，需要在测试中提供实现
set(UNIFIED_TEST_NAME "dfm-extension-tests")
if(TARGET ${UNIFIED_TEST_NAME})
    target_sources(${UNIFIED_TEST_NAME} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/dfm_extension_stub.cpp
    )
    message(STATUS "  ✅ 添加dfm-extension stub实现")
endif()

# 可选：打印组件摘要
dfm_print_component_summary(dfm-extension)

message(STATUS "✅ dfm-extension组件测试配置完成") 