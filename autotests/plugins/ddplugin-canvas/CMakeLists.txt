cmake_minimum_required(VERSION 3.10)

set(test_name "test-ddplugin-canvas")
set(plugin_path "${DFM_SOURCE_DIR}/plugins/desktop/ddplugin-canvas")

# Find test files
file(GLOB_RECURSE UT_CXX_FILE FILES_MATCHING PATTERN "*.cpp" "*.h")

# Define source files exactly as in the original plugin CMakeLists.txt
set(EXT_FILES
    ${DFM_SOURCE_DIR}/plugins/desktop/desktoputils/widgetutil.h
)

# Get all canvas plugin files (like the source does)
file(GLOB_RECURSE CANVAS_FILES
    "${plugin_path}/*.h"
    "${plugin_path}/*.cpp"
)

# Create test executable with precise file control
dfm_create_test_executable(${test_name}
    SOURCES ${UT_CXX_FILE} ${CANVAS_FILES} ${EXT_FILES}
)

# Apply plugin-specific configuration using shared dependencies
dfm_configure_plugin_dependencies(${test_name} "ddplugin-canvas" ${plugin_path})

# Include necessary paths to resolve relative includes
target_include_directories(${test_name} PRIVATE 
    "${plugin_path}"
    "${DFM_SOURCE_DIR}/plugins/desktop"
    "${CMAKE_CURRENT_SOURCE_DIR}"
)

message(STATUS "DFM: Created enhanced desktop canvas plugin test: ${test_name}") 