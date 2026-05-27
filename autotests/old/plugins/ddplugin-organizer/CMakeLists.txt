cmake_minimum_required(VERSION 3.10)

set(test_name "test-ddplugin-organizer")
set(plugin_path "${DFM_SOURCE_DIR}/plugins/desktop/ddplugin-organizer")

# Find test files
file(GLOB_RECURSE UT_CXX_FILE FILES_MATCHING PATTERN "*.cpp" "*.h")

# Define source files exactly as in the original plugin CMakeLists.txt
set(EXT_FILES
    ${DFM_SOURCE_DIR}/plugins/desktop/desktoputils/widgetutil.h
)

file(GLOB_RECURSE CORE_FILES
    "${plugin_path}/*.h"
    "${plugin_path}/*.cpp"
)

# Handle QRC resources like the source does
set(QRC_FILE
    ${plugin_path}/resources/images.qrc
)

qt_add_resources(QRC_RESOURCES ${QRC_FILE})

# Create test executable with precise file control
dfm_create_test_executable(${test_name}
    SOURCES ${UT_CXX_FILE} ${CORE_FILES} ${EXT_FILES} ${QRC_RESOURCES}
)

# Apply plugin-specific configuration using shared dependencies
dfm_configure_plugin_dependencies(${test_name} "ddplugin-organizer" ${plugin_path})

# Include necessary paths to resolve relative includes
target_include_directories(${test_name} PRIVATE 
    "${plugin_path}"
    "${DFM_SOURCE_DIR}/plugins/desktop"
    "${CMAKE_CURRENT_SOURCE_DIR}"
)

message(STATUS "DFM: Created enhanced desktop organizer plugin test: ${test_name}") 