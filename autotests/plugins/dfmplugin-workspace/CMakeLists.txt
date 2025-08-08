cmake_minimum_required(VERSION 3.10)

set(test_name "test-dfmplugin-workspace")
set(plugin_path "${DFM_SOURCE_DIR}/plugins/filemanager/dfmplugin-workspace")

# Find test files
file(GLOB_RECURSE UT_CXX_FILE FILES_MATCHING PATTERN "*.cpp" "*.h")

# Get all worksapce plugin files (like the source does)
file(GLOB_RECURSE WORKSPACE_FILES
    "${plugin_path}/*.h"
    "${plugin_path}/*.cpp"
)

# Create test executable with precise file control
dfm_create_test_executable(${test_name}
    SOURCES ${UT_CXX_FILE} ${WORKSPACE_FILES}
)

# Apply plugin-specific configuration using shared dependencies
dfm_configure_plugin_dependencies(${test_name} "dfmplugin-workspace" ${plugin_path})

# Define macro to disable Qt private class usage in unit tests
target_compile_definitions(${test_name} PRIVATE DFM_UNIT_TEST_DISABLE_QT_PRIVATE)

# Include necessary paths to resolve relative includes
target_include_directories(${test_name} PRIVATE
    "${plugin_path}"
    "${DFM_SOURCE_DIR}/plugins/filemanager"
    "${CMAKE_CURRENT_SOURCE_DIR}"
)

message(STATUS "DFM: Created enhanced filemanager workspace plugin test: ${test_name}")
