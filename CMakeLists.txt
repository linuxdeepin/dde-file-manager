cmake_minimum_required(VERSION 3.10)

project(filemanager)

# CMake modules
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# Include common DFM configuration
include(DFMCommon)

# Initialize common environment
dfm_init_common()

# Print build information
message(STATUS "=== DDE File Manager Build ===")
message(STATUS "Build mode: ${CMAKE_BUILD_TYPE}")
message(STATUS "Standalone test: ${DFM_STANDALONE_TEST}")
message(STATUS "Project root: ${DFM_PROJECT_ROOT}")
message(STATUS "Source directory: ${DFM_SOURCE_DIR}")
message(STATUS "Include directory: ${DFM_INCLUDE_DIR}")
message(STATUS "CMAKE_CXX_FLAGS: ${CMAKE_CXX_FLAGS}")
message(STATUS "==============================")

# options
option(OPT_DISABLE_QDEBUG "Disable Debug Macro" OFF)
option(OPT_ENABLE_BUILD_DOCS "Build develop documents" OFF)
option(OPT_ENABLE_BUILD_UT "Build unit tests" ON)


include(install_dconfig)
include(install_dbus_service)
include(GNUInstallDirs)
include(DFMInstallDirs)

# Skip build rpath for reproducible release build
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    set(CMAKE_SKIP_BUILD_RPATH TRUE)
endif()

# Enable position-independent code for improved security (ASLR)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# sub directories
add_subdirectory(src/apps)
add_subdirectory(src/dfm-base)
add_subdirectory(src/dfm-extension)
add_subdirectory(src/dfm-framework)
add_subdirectory(src/plugins)
add_subdirectory(src/external)
add_subdirectory(src/tools)
add_subdirectory(src/services)

# docs
if(OPT_ENABLE_BUILD_DOCS)
    find_package(Doxygen)
    if (DOXYGEN_FOUND)
        add_subdirectory(docs/api)
    endif()
endif()

# test
if(OPT_ENABLE_BUILD_UT)
    message(STATUS "Enable testing: ${BUILD_TESTING}")
    if(BUILD_TESTING)
        enable_testing()
        add_subdirectory(autotests)
    endif()
endif()

