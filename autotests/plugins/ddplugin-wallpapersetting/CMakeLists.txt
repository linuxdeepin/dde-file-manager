cmake_minimum_required(VERSION 3.10)

set(test_name "test-ddplugin-wallpapersetting")
set(plugin_path "${DFM_SOURCE_DIR}/plugins/desktop/ddplugin-wallpapersetting")

# Find test files
file(GLOB_RECURSE UT_CXX_FILE FILES_MATCHING PATTERN "*.cpp" "*.h")

# Define source files exactly as in the original plugin CMakeLists.txt
file(GLOB_RECURSE SRC_FILES
    "${plugin_path}/*.h"
    "${plugin_path}/*.cpp"
)

set(EXT_FILES
    ${DFM_SOURCE_DIR}/plugins/desktop/desktoputils/widgetutil.h
)

# Handle QRC resources like the source does
set(QRCS
    ${plugin_path}/images.qrc
)

# Create test executable with precise file control
dfm_create_test_executable(${test_name}
    SOURCES ${UT_CXX_FILE} ${SRC_FILES} ${EXT_FILES} ${QRCS}
)

# Apply plugin-specific configuration using shared dependencies
dfm_configure_plugin_dependencies(${test_name} "ddplugin-wallpapersetting" ${plugin_path})

# Include necessary paths to resolve relative includes
target_include_directories(${test_name} PRIVATE 
    "${plugin_path}"
    "${DFM_SOURCE_DIR}/plugins/desktop"
    "${CMAKE_CURRENT_SOURCE_DIR}"
)

message(STATUS "DFM: Created enhanced desktop wallpapersetting plugin test: ${test_name}") 