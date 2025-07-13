# autotests/units/dfm-base/CMakeLists.txt - dfm-base component test configuration
# Using zero-intrusion architecture, only one line configuration needed

message(STATUS "Configuring dfm-base component test...")

# Core: Only one line configuration needed, auto-discover source files and create test target
dfm_create_component_test(dfm-base)

message(STATUS "✅ dfm-base component test configuration completed")
