# DFMTestUtils.cmake - Lightweight test utilities for DDE File Manager
# Tests link against pre-built shared library targets (e.g. DFM6::framework).
# No source recompilation. Coverage is handled by the run-ut.sh script only.

cmake_minimum_required(VERSION 3.10)

# ---------------------------------------------------------------------------
# dfm_setup_test_env() - Initialize GTest and testutils (cpp-stub / stub-ext)
# Call once from autotests/CMakeLists.txt before adding any test subdirectories.
# ---------------------------------------------------------------------------
function(dfm_setup_test_env)
    # GTest
    find_package(GTest REQUIRED)
    include_directories(${GTEST_INCLUDE_DIRS})
    link_libraries(GTest::gtest GTest::gtest_main pthread)

    # Qt Test (needed by dfm_test_main.h → QApplication)
    find_package(Qt6 COMPONENTS Test REQUIRED)

    # testutils stub headers + sources
    set(_stub_root "${DFM_3RDPARTY_DIR}/testutils")
    if(NOT EXISTS "${_stub_root}")
        message(WARNING "DFM: testutils not found at ${_stub_root}")
    else()
        file(GLOB _stub_src
            "${_stub_root}/cpp-stub/*.h"
            "${_stub_root}/cpp-stub/*.hpp"
            "${_stub_root}/stub-ext/*.h"
            "${_stub_root}/stub-ext/*.cpp"
        )
        include_directories(
            "${_stub_root}/cpp-stub"
            "${_stub_root}/stub-ext"
            "${_stub_root}"
        )
        set(DFM_STUB_SOURCES ${_stub_src} CACHE INTERNAL "testutils source files")
    endif()

    # ASAN report directory
    set(_asan_dir "${CMAKE_BINARY_DIR}/asan-reports")
    file(MAKE_DIRECTORY "${_asan_dir}")
    add_definitions(-DDFM_ASAN_REPORT_DIR="${_asan_dir}")

    # Definitions for test builds
    add_definitions(-DDEBUG_STUB_INVOKE)

    message(STATUS "DFM: Test environment ready")
endfunction()

# ---------------------------------------------------------------------------
# dfm_add_test(<name> SOURCES ... LINK_LIBRARIES ...)
# Create a test executable, register it with CTest, and link testutils stubs.
#
# Required:
#   name           - test target name (also CTest test name)
#   SOURCES ...    - test source files
#
# Optional:
#   LINK_LIBRARIES ... - additional link libraries (e.g. DFM6::framework)
# ---------------------------------------------------------------------------
function(dfm_add_test name)
    cmake_parse_arguments(ARG "" "" "SOURCES;LINK_LIBRARIES" ${ARGN})

    if(NOT ARG_SOURCES)
        message(FATAL_ERROR "dfm_add_test(${name}): SOURCES is required")
    endif()

    # Collect sources: test files + stub files
    set(_all_sources ${ARG_SOURCES})
    if(DFM_STUB_SOURCES)
        list(APPEND _all_sources ${DFM_STUB_SOURCES})
    endif()

    add_executable(${name} ${_all_sources})
    target_compile_options(${name} PRIVATE -fno-access-control)
    target_include_directories(${name} PRIVATE
        ${CMAKE_SOURCE_DIR}/autotests   # for dfm_test_main.h, dfm_asan_helper.h
        ${DFM_SOURCE_DIR}               # for private headers (e.g. lifecycle/private/)
    )

    if(ARG_LINK_LIBRARIES)
        target_link_libraries(${name} PRIVATE ${ARG_LINK_LIBRARIES})
    endif()

    # Enable coverage flags if requested (only via run-ut.sh)
    if(DFM_ENABLE_COVERAGE)
        target_compile_options(${name} PRIVATE --coverage -O0 -fno-inline)
        target_link_libraries(${name} PRIVATE gcov)
    endif()

    add_test(NAME ${name} COMMAND ${name})
    message(STATUS "DFM: Added test ${name}")
endfunction()

message(STATUS "DFM: Test utilities module loaded")
