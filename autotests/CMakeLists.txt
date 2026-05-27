# DDE File Manager Unit Tests
# Tests link against pre-built shared library targets (DFM6::base, DFM6::framework, etc.)
# No source recompilation — only test .cpp files are compiled here.

include(DFMTestUtils)

# Initialize test environment (GTest, testutils stubs)
dfm_setup_test_env()

# Enable coverage flags if requested (only via run-ut.sh --coverage)
option(DFM_ENABLE_COVERAGE "Enable gcov coverage instrumentation" OFF)
if(DFM_ENABLE_COVERAGE)
    message(STATUS "DFM: Coverage instrumentation enabled")
endif()

# Test subdirectories
add_subdirectory(libs)
