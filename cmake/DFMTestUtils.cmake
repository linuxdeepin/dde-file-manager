# DFMTestUtils.cmake - Test utilities for DDE File Manager
# This module provides test-specific functionality and utilities

cmake_minimum_required(VERSION 3.10)

# Include plugin and service configuration modules
include(${CMAKE_CURRENT_LIST_DIR}/DFMPluginConfig.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/DFMServiceConfig.cmake)

# Global variable to store stub source files
set(CPP_STUB_SRC "" CACHE INTERNAL "Stub source files for testing")

# Global test compilation flags
set(DFM_TEST_CXX_FLAGS "" CACHE INTERNAL "Test-specific CXX flags")

# Function to setup test environment
function(dfm_setup_test_environment)
    message(STATUS "DFM: Setting up test environment...")
    
    # Test dependencies
    find_package(GTest REQUIRED)
    find_package(Qt6 COMPONENTS Test REQUIRED)

    # Include GTest
    include_directories(${GTEST_INCLUDE_DIRS})
    include_directories(${DFM_PROJECT_ROOT}/autotests)
    
    # Link libraries for all tests
    link_libraries(${GTEST_LIBRARIES} pthread stdc++fs Qt6::Test)
    
    # Test-specific definitions
    add_definitions(-DDEBUG_STUB_INVOKE)
    
    message(STATUS "DFM: Test environment configured")
endfunction()

# Function to setup test stub utilities
function(dfm_setup_test_stubs)
    dfm_get_test_utils_path(TEST_UTILS_PATH)
    
    # Debug output
    message(STATUS "DFM: Setting up test stubs...")
    message(STATUS "  - Test utils path: ${TEST_UTILS_PATH}")
    
    # Check if test utils path exists
    if(NOT EXISTS "${TEST_UTILS_PATH}")
        message(WARNING "DFM: Test utils path does not exist: ${TEST_UTILS_PATH}")
        return()
    endif()
    
    # Define ASAN report absolute path
    set(ASAN_REPORT_DIR "${CMAKE_BINARY_DIR}/asan-reports")
    add_definitions(-DDFM_ASAN_REPORT_DIR="${ASAN_REPORT_DIR}")
    
    # Create ASAN report directory
    file(MAKE_DIRECTORY "${ASAN_REPORT_DIR}")
    message(STATUS "DFM: ASAN report directory: ${ASAN_REPORT_DIR}")
    
    # Stub source files
    file(GLOB STUB_SRC_FILES 
        "${TEST_UTILS_PATH}/cpp-stub/*.h"
        "${TEST_UTILS_PATH}/cpp-stub/*.hpp"
        "${TEST_UTILS_PATH}/stub-ext/*.h"
        "${TEST_UTILS_PATH}/stub-ext/*.cpp"
    )
    
    # Debug output
    message(STATUS "DFM: Found stub files:")
    foreach(stub_file ${STUB_SRC_FILES})
        message(STATUS "    ${stub_file}")
    endforeach()
    
    # Set global variable
    set(CPP_STUB_SRC ${STUB_SRC_FILES} CACHE INTERNAL "Stub source files for testing")
    
    # Include stub directories and ASAN helper
    include_directories(
        "${TEST_UTILS_PATH}/cpp-stub"
        "${TEST_UTILS_PATH}/stub-ext"
        "${TEST_UTILS_PATH}"
    )
    
    message(STATUS "DFM: Test stubs and ASAN helper configured")
endfunction()

# Function to setup coverage settings
function(dfm_setup_coverage)
    # Base test compilation flags - always needed for testing
    set(TEST_FLAGS "-fno-inline;-fno-access-control;-O0")
    
    if (CMAKE_BUILD_TYPE STREQUAL "Debug")
        set(CMAKE_VERBOSE_MAKEFILE ON)
        add_compile_definitions(ENABLE_TSAN_TOOL)
        
        message(STATUS "DFM: Debug build - enabling sanitizers")
        set(SANITIZER_FLAGS "-fsanitize=undefined,address,leak;-fno-omit-frame-pointer")
        list(APPEND TEST_FLAGS ${SANITIZER_FLAGS})
        
        # Set global flags for parent scope
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=undefined,address,leak -fno-omit-frame-pointer" PARENT_SCOPE)
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=undefined,address,leak -fno-omit-frame-pointer" PARENT_SCOPE)
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=undefined,address,leak -fno-omit-frame-pointer" PARENT_SCOPE)
    endif()
    
    # Coverage compilation flags
    list(APPEND TEST_FLAGS "-fprofile-arcs;-ftest-coverage;-lgcov")
    
    # Store test flags globally
    set(DFM_TEST_CXX_FLAGS ${TEST_FLAGS} CACHE INTERNAL "Test-specific CXX flags")
    
    # Set global flags for parent scope
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-inline -fno-access-control -O0 -fprofile-arcs -ftest-coverage -lgcov" PARENT_SCOPE)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DQT_DEBUG" PARENT_SCOPE)
    
    message(STATUS "DFM: Coverage settings configured")
    message(STATUS "  - Test flags: ${TEST_FLAGS}")
endfunction()

# Function to create a DFM test executable
function(dfm_create_test_executable test_name)
    set(options "")
    set(oneValueArgs "")
    set(multiValueArgs SOURCES HEADERS DEPENDENCIES LINK_LIBRARIES)
    cmake_parse_arguments(TEST "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})
    
    # Debug output
    message(STATUS "DFM: Creating test executable: ${test_name}")
    
    # Collect all source files
    set(ALL_SOURCES ${TEST_SOURCES})
    
    # Add headers if provided
    if(TEST_HEADERS)
        list(APPEND ALL_SOURCES ${TEST_HEADERS})
    endif()
    
    # Add stub sources if available
    if(CPP_STUB_SRC)
        list(APPEND ALL_SOURCES ${CPP_STUB_SRC})
    endif()

    # Create executable
    add_executable(${test_name} ${ALL_SOURCES})
    
    # Apply test-specific compilation flags to this target
    if(DFM_TEST_CXX_FLAGS)
        target_compile_options(${test_name} PRIVATE ${DFM_TEST_CXX_FLAGS})
        message(STATUS "DFM: Applied test flags to ${test_name}: ${DFM_TEST_CXX_FLAGS}")
    endif()
    
    # Setup include directories
    target_include_directories(${test_name} PRIVATE
        ${DFM_INCLUDE_DIR}
        ${DFM_SOURCE_DIR}
    )
    
    # Link libraries
    if(TEST_LINK_LIBRARIES)
        target_link_libraries(${test_name} PRIVATE ${TEST_LINK_LIBRARIES})
    endif()
    
    # Add sanitizer and coverage link libraries if in debug mode
    if (CMAKE_BUILD_TYPE STREQUAL "Debug")
        target_link_libraries(${test_name} PRIVATE 
            -fsanitize=undefined,address,leak 
            -fprofile-arcs 
            -ftest-coverage 
            -lgcov
        )
    else()
        target_link_libraries(${test_name} PRIVATE 
            -fprofile-arcs 
            -ftest-coverage 
            -lgcov
        )
    endif()
    
    # Add test
    add_test(NAME ${test_name} COMMAND ${test_name})
    
    message(STATUS "DFM: Created test executable: ${test_name}")
endfunction()

# Function to create a DFM library test
function(dfm_create_library_test lib_name)
    set(test_name "test-${lib_name}")

    # Determine source paths based on library name
    if(lib_name STREQUAL "dfm-base")
        set(SRC_HEADER_PATH "${DFM_INCLUDE_DIR}/dfm-base/")
        set(SRC_PATH "${DFM_SOURCE_DIR}/dfm-base/")
    elseif(lib_name STREQUAL "dfm-framework")
        set(SRC_HEADER_PATH "${DFM_INCLUDE_DIR}/dfm-framework/")
        set(SRC_PATH "${DFM_SOURCE_DIR}/dfm-framework/")
    elseif(lib_name STREQUAL "dfm-extension")
        set(SRC_HEADER_PATH "${DFM_INCLUDE_DIR}/dfm-extension/")
        set(SRC_PATH "${DFM_SOURCE_DIR}/dfm-extension/")
    else()
        message(FATAL_ERROR "DFM: Unknown library name: ${lib_name}")
    endif()
    
    # Get library dependencies using unified configuration
    dfm_get_library_test_dependencies(${lib_name} EXTRA_LIBS)
    
    # Find test files
    file(GLOB_RECURSE UT_CXX_FILE FILES_MATCHING PATTERN "*.cpp" "*.h")
    
    # Find source files
    file(GLOB_RECURSE HEADER_FILES CONFIGURE_DEPENDS "${SRC_HEADER_PATH}/*")
    file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS
        "${SRC_PATH}/*.h"
        "${SRC_PATH}/*.cpp"
    )
    
    # Create test executable
    dfm_create_test_executable(${test_name}
        SOURCES ${UT_CXX_FILE} ${SRC_FILES}
        HEADERS ${HEADER_FILES}
        LINK_LIBRARIES ${EXTRA_LIBS}
    )

    # Special handling for dfm-base - configure using unified function
    if(lib_name STREQUAL "dfm-base")
        # Set required variables for dfm-base configuration
        set(QT_VERSION_MAJOR 6)
        set(DTK_VERSION_MAJOR 6)
        
        # Configure the test target using the same function as the library
        dfm_configure_base_library(${test_name})
        
        # Additional test-specific definitions
        add_compile_definitions(QT_NO_SIGNALS_SLOTS_KEYWORDS)
        add_compile_definitions(THUMBNAIL_TOOL_DIR="${DFM_THUMBNAIL_TOOL}")
    endif()
    
    target_include_directories(${test_name} PRIVATE "${SRC_PATH}")

    message(STATUS "DFM: Created library test: ${test_name} for ${lib_name}")
endfunction()

# Function to create a DFM plugin test (backward compatibility)
function(dfm_create_plugin_test plugin_name plugin_path)
    # Call the enhanced version for backward compatibility
    dfm_create_plugin_test_enhanced(${plugin_name} ${plugin_path})
endfunction()

# Enhanced function to create a DFM plugin test with intelligent configuration
function(dfm_create_plugin_test_enhanced plugin_name plugin_path)
    set(test_name "test-${plugin_name}")
    
    message(STATUS "DFM: Creating enhanced plugin test: ${test_name} for ${plugin_name}")
    
    # Find test files
    file(GLOB_RECURSE UT_CXX_FILE FILES_MATCHING PATTERN "*.cpp" "*.h")
    
    # Find plugin source files
    file(GLOB_RECURSE SRC_FILES FILES_MATCHING PATTERN 
        "${plugin_path}/*.cpp" 
        "${plugin_path}/*.h"
    )
    
    # Create test executable with basic configuration
    dfm_create_test_executable(${test_name}
        SOURCES ${UT_CXX_FILE} ${SRC_FILES}
    )
    
    # Apply plugin-specific configuration using new system
    dfm_configure_plugin_dependencies(${test_name} ${plugin_name} ${plugin_path})
    
    # Include plugin path
    target_include_directories(${test_name} PRIVATE "${plugin_path}")
    
    message(STATUS "DFM: Created enhanced plugin test: ${test_name} for ${plugin_name}")
endfunction()

# Function to create a DFM service test (backward compatibility)
function(dfm_create_service_test service_name service_path)
    set(test_name "test-${service_name}")
    
    # Find test files
    file(GLOB_RECURSE UT_CXX_FILE FILES_MATCHING PATTERN "*.cpp" "*.h")
    
    # Find service source files (exclude main.cpp to avoid conflicts)
    file(GLOB_RECURSE SRC_FILES FILES_MATCHING PATTERN 
        "${service_path}/*.cpp" 
        "${service_path}/*.h"
    )
    
    # Remove main.cpp from service sources to avoid main function conflicts
    list(FILTER SRC_FILES EXCLUDE REGEX ".*/main\\.cpp$")
    
    # Create test executable with default configuration
    dfm_create_test_executable(${test_name}
        SOURCES ${UT_CXX_FILE} ${SRC_FILES}
    )
    
    # Apply default service configuration
    dfm_apply_default_service_config(${test_name})
    
    # Include service path
    target_include_directories(${test_name} PRIVATE "${service_path}")
    
    message(STATUS "DFM: Created service test: ${test_name} for ${service_name}")
endfunction()

# Function to create a DFM service test with enhanced configuration
function(dfm_create_service_test_enhanced service_name service_path)
    set(test_name "test-${service_name}")
    
    # Find test files
    file(GLOB_RECURSE UT_CXX_FILE FILES_MATCHING PATTERN "*.cpp" "*.h")
    
    # Find service source files (exclude main.cpp to avoid conflicts)
    file(GLOB_RECURSE SRC_FILES FILES_MATCHING PATTERN 
        "${service_path}/*.cpp" 
        "${service_path}/*.h"
    )
    
    # Remove main.cpp from service sources to avoid main function conflicts
    list(FILTER SRC_FILES EXCLUDE REGEX ".*/main\\.cpp$")
    
    # Create test executable
    dfm_create_test_executable(${test_name}
        SOURCES ${UT_CXX_FILE} ${SRC_FILES}
    )
    
    # Apply service-specific configuration
    dfm_configure_service(${test_name} ${service_name} ${service_path})
    
    # Include service path
    target_include_directories(${test_name} PRIVATE "${service_path}")
    
    message(STATUS "DFM: Created enhanced service test: ${test_name} for ${service_name}")
endfunction()

# Function to setup DFM library targets for testing
function(dfm_setup_library_targets)
    if(DFM_STANDALONE_TEST)
        # In standalone mode, try to find or create library targets
        
        # DFM6::base
        if(NOT TARGET DFM6::base)
            find_package(dfm6-base QUIET)
            if(dfm6-base_FOUND)
                message(STATUS "DFM: Found dfm6-base package")
                # Create alias if the package creates DFM6::dfm6-base but we need DFM6::base
                if(TARGET DFM6::dfm6-base AND NOT TARGET DFM6::base)
                    add_library(DFM6::base ALIAS DFM6::dfm6-base)
                    message(STATUS "DFM: Created alias DFM6::base -> DFM6::dfm6-base")
                endif()
            else()
                # Try to find the library manually
                find_library(DFM6_BASE_LIB NAMES dfm6-base libdfm6-base)
                if(DFM6_BASE_LIB)
                    add_library(DFM6::base SHARED IMPORTED)
                    set_target_properties(DFM6::base PROPERTIES
                        IMPORTED_LOCATION ${DFM6_BASE_LIB}
                        INTERFACE_INCLUDE_DIRECTORIES ${DFM_INCLUDE_DIR}
                    )
                    message(STATUS "DFM: Created imported target DFM6::base")
                else()
                    message(WARNING "DFM: Could not find dfm6-base library")
                endif()
            endif()
        endif()
        
        # DFM6::framework
        if(NOT TARGET DFM6::framework)
            find_package(dfm6-framework QUIET)
            if(dfm6-framework_FOUND)
                message(STATUS "DFM: Found dfm6-framework package")
                # Create target since dfm6-framework package doesn't create one
                find_library(DFM6_FRAMEWORK_LIB NAMES dfm6-framework libdfm6-framework)
                if(DFM6_FRAMEWORK_LIB)
                    add_library(DFM6::framework SHARED IMPORTED)
                    set_target_properties(DFM6::framework PROPERTIES
                        IMPORTED_LOCATION ${DFM6_FRAMEWORK_LIB}
                        INTERFACE_INCLUDE_DIRECTORIES ${DFM_INCLUDE_DIR}
                    )
                    message(STATUS "DFM: Created imported target DFM6::framework")
                else()
                    message(WARNING "DFM: Could not find dfm6-framework library")
                endif()
            else()
                find_library(DFM6_FRAMEWORK_LIB NAMES dfm6-framework libdfm6-framework)
                if(DFM6_FRAMEWORK_LIB)
                    add_library(DFM6::framework SHARED IMPORTED)
                    set_target_properties(DFM6::framework PROPERTIES
                        IMPORTED_LOCATION ${DFM6_FRAMEWORK_LIB}
                        INTERFACE_INCLUDE_DIRECTORIES ${DFM_INCLUDE_DIR}
                    )
                    message(STATUS "DFM: Created imported target DFM6::framework")
                else()
                    message(WARNING "DFM: Could not find dfm6-framework library")
                endif()
            endif()
        endif()
        
        # DFM::extension
        if(NOT TARGET DFM::extension)
            find_package(dfm-extension QUIET)
            if(dfm-extension_FOUND)
                message(STATUS "DFM: Found dfm-extension package")
                # Even if package is found, we need to create the target if it doesn't exist
                # because the package config file only sets variables, not targets
                if(NOT TARGET DFM::extension)
                    if(dfm-extension_LIBRARIES)
                        # The dfm-extension_LIBRARIES might be just the library name, not the full path
                        # We need to find the actual library file
                        if(dfm-extension_LIBRARIES STREQUAL "dfm-extension")
                            find_library(DFM_EXTENSION_ACTUAL_LIB NAMES dfm-extension libdfm-extension)
                            if(DFM_EXTENSION_ACTUAL_LIB)
                                add_library(DFM::extension SHARED IMPORTED)
                                set_target_properties(DFM::extension PROPERTIES
                                    IMPORTED_LOCATION ${DFM_EXTENSION_ACTUAL_LIB}
                                    INTERFACE_INCLUDE_DIRECTORIES ${dfm-extension_INCLUDE_DIR}
                                )
                                message(STATUS "DFM: Created imported target DFM::extension from package with actual library: ${DFM_EXTENSION_ACTUAL_LIB}")
                            endif()
                        else()
                            # If it's already a full path, use it directly
                            add_library(DFM::extension SHARED IMPORTED)
                            set_target_properties(DFM::extension PROPERTIES
                                IMPORTED_LOCATION ${dfm-extension_LIBRARIES}
                                INTERFACE_INCLUDE_DIRECTORIES ${dfm-extension_INCLUDE_DIR}
                            )
                            message(STATUS "DFM: Created imported target DFM::extension from package")
                        endif()
                    endif()
                endif()
            else()
                find_library(DFM_EXTENSION_LIB NAMES dfm-extension libdfm-extension)
                if(DFM_EXTENSION_LIB)
                    add_library(DFM::extension SHARED IMPORTED)
                    set_target_properties(DFM::extension PROPERTIES
                        IMPORTED_LOCATION ${DFM_EXTENSION_LIB}
                        INTERFACE_INCLUDE_DIRECTORIES ${DFM_INCLUDE_DIR}
                    )
                    message(STATUS "DFM: Created imported target DFM::extension")
                else()
                    message(WARNING "DFM: Could not find dfm-extension library")
                endif()
            endif()
        endif()
        
    else()
        # In integrated mode, targets should already exist
        message(STATUS "DFM: Using integrated build library targets")
    endif()
endfunction()

# Main function to initialize test environment
function(dfm_init_test_environment)
    message(STATUS "DFM: Initializing test environment...")
    
    dfm_setup_test_environment()
    dfm_setup_test_stubs()
    dfm_setup_coverage()
    dfm_setup_library_targets()
    
    message(STATUS "DFM: Test environment initialized successfully")
endfunction()

message(STATUS "DFM: Test utilities module loaded") 
