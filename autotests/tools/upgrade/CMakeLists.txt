cmake_minimum_required(VERSION 3.10)

set(test_name "test-upgrade")

# Find test files
file(GLOB_RECURSE UT_CXX_FILE FILES_MATCHING PATTERN "*.cpp" "*.h")

# Find upgrade tool source files
file(GLOB_RECURSE SRC_FILES FILES_MATCHING PATTERN 
    "${DFM_SOURCE_DIR}/tools/upgrade/*.cpp" 
    "${DFM_SOURCE_DIR}/tools/upgrade/*.h"
)

configure_file(
    "${APP_SOURCE_DIR}/config.h.in"
    "${CMAKE_CURRENT_BINARY_DIR}/config.h"
    )

# Create test executable with basic configuration
dfm_create_test_executable(${test_name}
    SOURCES ${UT_CXX_FILE} ${SRC_FILES}
    LINK_LIBRARIES 
        DFM6::base
        DFM6::framework
        Qt6::Core
)

# Include upgrade tool path
target_include_directories(${test_name} PRIVATE "${DFM_SOURCE_DIR}/tools/upgrade")

message(STATUS "DFM: Created upgrade tool test: ${test_name}") 
