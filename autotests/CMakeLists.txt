# DDE File Manager Unit Tests
# Tests link against pre-built shared library targets (DFM6::base, DFM6::framework, etc.)
# No source recompilation — only test .cpp files are compiled here.

include(DFMTestUtils)
include(DFMPluginConfig)

# Initialize test environment (GTest, testutils stubs)
dfm_setup_test_env()

# Enable coverage flags if requested (only via run-ut.sh --coverage)
option(DFM_ENABLE_COVERAGE "Enable gcov coverage instrumentation" OFF)
if(DFM_ENABLE_COVERAGE)
    message(STATUS "DFM: Coverage instrumentation enabled")
endif()

# Per-category test switches (default ON; turn off to skip a category)
option(OPT_ENABLE_BUILD_UT_LIBS     "Build libs unit tests"      ON)
option(OPT_ENABLE_BUILD_UT_SERVICES "Build services unit tests"  ON)
option(OPT_ENABLE_BUILD_UT_PLUGINS  "Build plugins unit tests"   ON)
option(OPT_ENABLE_BUILD_UT_APPS     "Build apps unit tests"      ON)
option(OPT_ENABLE_BUILD_UT_TOOLS    "Build tools unit tests"     ON)

if(OPT_ENABLE_BUILD_UT_LIBS)
    add_subdirectory(libs)
endif()

if(OPT_ENABLE_BUILD_UT_SERVICES)
    add_subdirectory(services)
endif()

if(OPT_ENABLE_BUILD_UT_PLUGINS)
    add_subdirectory(plugins)
endif()

if(OPT_ENABLE_BUILD_UT_APPS)
    add_subdirectory(apps)
endif()

if(OPT_ENABLE_BUILD_UT_TOOLS)
    add_subdirectory(tools)
endif()
