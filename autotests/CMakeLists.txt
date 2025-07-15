# DDE File Manager Unit Tests
# This CMakeLists.txt supports both standalone and integrated build modes

cmake_minimum_required(VERSION 3.10)

# Set project name
project(ut-file-manager)

# Include common DFM modules
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../cmake")
include(DFMCommon)
include(DFMTestUtils)

# Initialize common environment
dfm_init_common()

# Initialize test environment
dfm_init_test_environment()

# Print build mode information
message(STATUS "=== DDE File Manager Unit Tests ===")
message(STATUS "Build mode: ${CMAKE_BUILD_TYPE}")
message(STATUS "Standalone test: ${DFM_STANDALONE_TEST}")
message(STATUS "Project root: ${DFM_PROJECT_ROOT}")
message(STATUS "Source directory: ${DFM_SOURCE_DIR}")
message(STATUS "Include directory: ${DFM_INCLUDE_DIR}")
message(STATUS "===================================")

# Enable testing
enable_testing()

# Add subdirectories
add_subdirectory(libs)
add_subdirectory(plugins)

# Optional: add other test directories if they exist
# add_subdirectory(apps)
# add_subdirectory(external)
# add_subdirectory(tools)
