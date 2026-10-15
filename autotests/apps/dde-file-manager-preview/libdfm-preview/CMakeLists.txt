cmake_minimum_required(VERSION 3.10)

# Unit tests for libdfm-preview: recompile the library sources into the test
# binary so stub-ext can access private members (see autotests/README.md).

# --- a tiny real Qt preview plugin that PreviewPluginLoader can load from disk
set(_fake_plugin_dir "${CMAKE_CURRENT_BINARY_DIR}/fakeplugins")
add_library(cov-fake-preview-plugin MODULE fakeplugin/test_fakepreviewplugin.cpp)
set_target_properties(cov-fake-preview-plugin PROPERTIES
    AUTOMOC ON
    PREFIX ""
    LIBRARY_OUTPUT_DIRECTORY "${_fake_plugin_dir}")
target_include_directories(cov-fake-preview-plugin PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/fakeplugin")
target_link_libraries(cov-fake-preview-plugin PRIVATE
    DFM6::base
    DFM6::framework
    Qt6::Widgets)

# --- the test binary itself
set(_lib_dir "${DFM_SOURCE_DIR}/apps/dde-file-manager-preview/libdfm-preview")

file(GLOB _ut_files CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/test_*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/main.cpp")

file(GLOB_RECURSE _lib_files CONFIGURE_DEPENDS
    "${_lib_dir}/*.cpp"
    "${_lib_dir}/*.h")

find_package(Qt6 COMPONENTS Widgets Network REQUIRED)
find_package(Dtk6 COMPONENTS Widget Gui REQUIRED)

dfm_create_test_executable(ut-libdfm-preview
    SOURCES ${_ut_files} ${_lib_files}
        # compiled in only for AUTOMOC: CovPreview/CovPreviewPlugin are Q_OBJECT
        "${CMAKE_CURRENT_SOURCE_DIR}/fakeplugin/test_fakepreviewplugin.h"
    LINK_LIBRARIES DFM6::base DFM6::framework Qt6::Widgets Qt6::Network Dtk6::Widget Dtk6::Gui
)

add_dependencies(ut-libdfm-preview cov-fake-preview-plugin)

target_include_directories(ut-libdfm-preview PRIVATE
    "${_lib_dir}"
    "${CMAKE_CURRENT_SOURCE_DIR}/fakeplugin"
    "${DFM_PREVIEW_PLUGIN_DIR}")

target_compile_definitions(ut-libdfm-preview PRIVATE
    DFM_PLUGIN_PREVIEW_DIR="${DFM_PLUGIN_PREVIEW_DIR}"
    VERSION="1.8.2"
    UT_FAKE_PLUGIN_DIR="${_fake_plugin_dir}"
)
