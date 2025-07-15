# DFMTestUtils.cmake - Test utilities for DDE File Manager
# This module provides test-specific functionality and utilities

cmake_minimum_required(VERSION 3.10)

# Function to setup test environment
function(dfm_setup_test_environment)
    message(STATUS "DFM: Setting up test environment...")
    
    # Test dependencies
    find_package(GTest REQUIRED)
    find_package(Qt6 COMPONENTS Test REQUIRED)
    
    # Include GTest
    include_directories(${GTEST_INCLUDE_DIRS})
    
    # Link libraries for all tests
    link_libraries(${GTEST_LIBRARIES} pthread stdc++fs Qt6::Test)
    
    # Test-specific definitions
    add_definitions(-DDEBUG_STUB_INVOKE)
    
    message(STATUS "DFM: Test environment configured")
endfunction()

# Function to setup test stub utilities
function(dfm_setup_test_stubs)
    dfm_get_test_utils_path(TEST_UTILS_PATH)
    
    # Stub source files
    file(GLOB CPP_STUB_SRC 
        "${TEST_UTILS_PATH}/cpp-stub/*.h"
        "${TEST_UTILS_PATH}/cpp-stub/*.hpp"
        "${TEST_UTILS_PATH}/stub-ext/*.h"
        "${TEST_UTILS_PATH}/stub-ext/*.cpp"
    )
    
    # Include stub directories
    include_directories(
        "${TEST_UTILS_PATH}/cpp-stub"
        "${TEST_UTILS_PATH}/stub-ext"
    )
    
    # Make stub sources available to parent scope
    set(CPP_STUB_SRC ${CPP_STUB_SRC} PARENT_SCOPE)
    
    message(STATUS "DFM: Test stubs configured")
    message(STATUS "  - Stub path: ${TEST_UTILS_PATH}")
endfunction()

# Function to setup coverage settings
function(dfm_setup_coverage)
    if (CMAKE_BUILD_TYPE STREQUAL "Debug")
        set(CMAKE_VERBOSE_MAKEFILE ON)
        add_compile_definitions(ENABLE_TSAN_TOOL)
        
        message(STATUS "DFM: Debug build - enabling sanitizers")
        set(SANITIZER_FLAGS "-fsanitize=undefined,address,leak -fno-omit-frame-pointer")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${SANITIZER_FLAGS}")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${SANITIZER_FLAGS}")
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${SANITIZER_FLAGS}")
        
        # Make flags available to parent scope
        set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} PARENT_SCOPE)
        set(CMAKE_C_FLAGS ${CMAKE_C_FLAGS} PARENT_SCOPE)
        set(CMAKE_EXE_LINKER_FLAGS ${CMAKE_EXE_LINKER_FLAGS} PARENT_SCOPE)
    endif()
    
    # Coverage compilation flags
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-inline -fno-access-control -O0 -fprofile-arcs -ftest-coverage -lgcov")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DQT_DEBUG")
    
    # Make flags available to parent scope
    set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} PARENT_SCOPE)
    set(CMAKE_CXX_FLAGS_DEBUG ${CMAKE_CXX_FLAGS_DEBUG} PARENT_SCOPE)
    
    message(STATUS "DFM: Coverage settings configured")
endfunction()

# Function to create a DFM test executable
function(dfm_create_test_executable test_name)
    set(options "")
    set(oneValueArgs "")
    set(multiValueArgs SOURCES HEADERS DEPENDENCIES LINK_LIBRARIES)
    cmake_parse_arguments(TEST "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})
    
    # Collect all source files
    set(ALL_SOURCES ${TEST_SOURCES} ${TEST_HEADERS} ${CPP_STUB_SRC})
    
    # Create executable
    add_executable(${test_name} ${ALL_SOURCES})
    
    # Setup include directories
    target_include_directories(${test_name} PRIVATE
        ${DFM_INCLUDE_DIR}
        ${DFM_SOURCE_DIR}
    )
    
    # Link libraries
    if(TEST_LINK_LIBRARIES)
        target_link_libraries(${test_name} PRIVATE ${TEST_LINK_LIBRARIES})
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
        set(EXTRA_LIBS Qt6::Core Qt6::Widgets Qt6::Gui Qt6::Concurrent Qt6::DBus Qt6::Sql Qt6::Network
                      Dtk6::Core Dtk6::Widget Dtk6::Gui dfm6-io dfm6-mount dfm6-burn
                      PkgConfig::mount PkgConfig::gio PkgConfig::X11 poppler-cpp ${LIBHEIF_LIBRARIES})
    elseif(lib_name STREQUAL "dfm-framework")
        set(SRC_HEADER_PATH "${DFM_INCLUDE_DIR}/dfm-framework/")
        set(SRC_PATH "${DFM_SOURCE_DIR}/dfm-framework/")
        set(EXTRA_LIBS Qt6::Core Qt6::Concurrent Dtk6::Core ${CMAKE_DL_LIBS})
    elseif(lib_name STREQUAL "dfm-extension")
        set(SRC_HEADER_PATH "${DFM_INCLUDE_DIR}/dfm-extension/")
        set(SRC_PATH "${DFM_SOURCE_DIR}/dfm-extension/")
        set(EXTRA_LIBS Qt6::Core ${CMAKE_DL_LIBS})
    else()
        message(FATAL_ERROR "DFM: Unknown library name: ${lib_name}")
    endif()
    
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
        SOURCES ${UT_CXX_FILE} ${SRC_FILES} ${HEADER_FILES}
        LINK_LIBRARIES ${EXTRA_LIBS}
    )
    
    # Special handling for dfm-base
    if(lib_name STREQUAL "dfm-base")
        # Add DBus interface
        qt6_add_dbus_interface(Qt6App_dbus
            ${DFM_DBUS_XML_DIR}/org.deepin.Filemanager.Daemon.DeviceManager.xml
            devicemanager_interface_qt6)
        target_sources(${test_name} PRIVATE ${Qt6App_dbus})
        
        # Additional definitions
        target_compile_definitions(${test_name} PRIVATE
            QT_NO_SIGNALS_SLOTS_KEYWORDS
            THUMBNAIL_TOOL_DIR="${DFM_THUMBNAIL_TOOL}"
            APPSHAREDIR="${CMAKE_INSTALL_PREFIX}/share/dde-file-manager"
        )
    endif()
    
    message(STATUS "DFM: Created library test: ${test_name} for ${lib_name}")
endfunction()

# Function to create a DFM plugin test
function(dfm_create_plugin_test plugin_name plugin_path)
    set(test_name "test-${plugin_name}")
    
    # Find test files
    file(GLOB_RECURSE UT_CXX_FILE FILES_MATCHING PATTERN "*.cpp" "*.h")
    
    # Find plugin source files
    file(GLOB_RECURSE SRC_FILES FILES_MATCHING PATTERN 
        "${plugin_path}/*.cpp" 
        "${plugin_path}/*.h"
    )
    
    # Create test executable
    dfm_create_test_executable(${test_name}
        SOURCES ${UT_CXX_FILE} ${SRC_FILES}
        LINK_LIBRARIES DFM6::base DFM6::framework Dtk6::Widget
    )
    
    # Include plugin path
    target_include_directories(${test_name} PRIVATE "${plugin_path}")
    
    message(STATUS "DFM: Created plugin test: ${test_name} for ${plugin_name}")
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
