cmake_minimum_required(VERSION 3.10)

set(test_name "test-ddplugin-background")
set(plugin_path "${DFM_SOURCE_DIR}/plugins/desktop/ddplugin-background")

# Find test files
file(GLOB_RECURSE UT_CXX_FILE FILES_MATCHING PATTERN "*.cpp" "*.h")

# Define source files exactly as in the original plugin CMakeLists.txt
file(GLOB_RECURSE SRC_FILES
    "${plugin_path}/*.h"
    "${plugin_path}/*.cpp"
)

# Delete files which are not needed on the current system (like the source does)
file(GLOB_RECURSE RM_SRC
    "${plugin_path}/backgroundwm.h"
    "${plugin_path}/backgroundwm.cpp"
)

list(REMOVE_ITEM SRC_FILES ${RM_SRC})

# Create test executable with precise file control
dfm_create_test_executable(${test_name}
    SOURCES ${UT_CXX_FILE} ${SRC_FILES}
)

# Apply plugin-specific configuration using shared dependencies
dfm_configure_plugin_dependencies(${test_name} "ddplugin-background" ${plugin_path})

# Include necessary paths to resolve relative includes
target_include_directories(${test_name} PRIVATE 
    "${plugin_path}"
    "${DFM_SOURCE_DIR}/plugins/desktop"
    "${CMAKE_CURRENT_SOURCE_DIR}"
)

message(STATUS "DFM: Created enhanced desktop background plugin test: ${test_name}") 