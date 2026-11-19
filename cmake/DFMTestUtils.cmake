# DFMTestUtils.cmake - Lightweight test utilities for DDE File Manager
# Tests link against pre-built shared library targets (e.g. DFM6::framework).
# No source recompilation. Coverage is handled by the run-ut.sh script only.

cmake_minimum_required(VERSION 3.10)

# ---------------------------------------------------------------------------
# dfm_setup_test_env() - Initialize GTest and testutils (cpp-stub / stub-ext)
# Call once from autotests/CMakeLists.txt before adding any test subdirectories.
# ---------------------------------------------------------------------------
function(dfm_setup_test_env)
    # Reset suite-to-source mapping file (consumed by autotests/run-ut.sh
    # for incremental CI: changed files -> affected test suites)
    file(WRITE "${CMAKE_BINARY_DIR}/ut-suite-map.tsv" "# suite\tsource-dir\n")
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
#   COVERS ...         - source dirs (relative to CMAKE_SOURCE_DIR) this suite
#                        covers; recorded in build/ut-suite-map.tsv so
#                        run-ut.sh can map changed files to suites
# ---------------------------------------------------------------------------
function(dfm_add_test name)
    cmake_parse_arguments(ARG "" "" "SOURCES;LINK_LIBRARIES;COVERS" ${ARGN})

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

    # Record suite mapping for incremental CI (run-ut.sh):
    #   suite -> its own test dir + every COVERS source dir
    # The test dir is derived from the first real test source (skipping the
    # shared main.cpp / stub files) so per-subdir suites map precisely.
    set(_map_file "${CMAKE_BINARY_DIR}/ut-suite-map.tsv")
    set(_test_dir "")
    foreach(_s ${ARG_SOURCES})
        if(NOT IS_ABSOLUTE "${_s}")
            get_filename_component(_s "${_s}" ABSOLUTE)
        endif()
        get_filename_component(_fname "${_s}" NAME)
        if(_fname STREQUAL "main.cpp" OR _fname STREQUAL "dfm_extension_stub.cpp")
            continue()
        endif()
        if(DFM_STUB_SOURCES AND _s IN_LIST DFM_STUB_SOURCES)
            continue()
        endif()
        get_filename_component(_test_dir "${_s}" DIRECTORY)
        break()
    endforeach()
    if("${_test_dir}" STREQUAL "")
        set(_test_dir "${CMAKE_CURRENT_SOURCE_DIR}")
    endif()
    file(RELATIVE_PATH _test_dir_rel "${CMAKE_SOURCE_DIR}" "${_test_dir}")
    file(APPEND "${_map_file}" "${name}\t${_test_dir_rel}\n")
    foreach(_cover ${ARG_COVERS})
        if(IS_ABSOLUTE "${_cover}")
            file(RELATIVE_PATH _cover_rel "${CMAKE_SOURCE_DIR}" "${_cover}")
        else()
            set(_cover_rel "${_cover}")
        endif()
        file(APPEND "${_map_file}" "${name}\t${_cover_rel}\n")
    endforeach()

    message(STATUS "DFM: Added test ${name}")
endfunction()

message(STATUS "DFM: Test utilities module loaded")
