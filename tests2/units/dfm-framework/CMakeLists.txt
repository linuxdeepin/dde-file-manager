# tests2/units/dfm-framework/CMakeLists.txt - dfm-framework组件测试配置
# 使用零侵入架构，仅需一行配置即可完成整个组件的测试设置

message(STATUS "配置dfm-framework组件测试...")

# 核心：仅需一行配置，自动发现源文件并创建测试目标
dfm_create_component_test(dfm-framework)

# 可选：打印组件摘要
dfm_print_component_summary(dfm-framework)

message(STATUS "✅ dfm-framework组件测试配置完成")