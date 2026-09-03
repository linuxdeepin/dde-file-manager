cmake_minimum_required(VERSION 3.10)

# Unit tests for text-preview: recompile plugin sources into the test binary
# so stub-ext can access private members (see autotests/README.md).
set(_preview_dir "${DFM_PREVIEW_PLUGIN_DIR}/text-preview")

file(GLOB_RECURSE _ut_files
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h")

file(GLOB_RECURSE _plugin_files
    "${_preview_dir}/*.cpp"
    "${_preview_dir}/*.h")

find_package(Qt6 COMPONENTS Widgets REQUIRED)
find_package(KF6SyntaxHighlighting REQUIRED)

dfm_create_test_executable(ut-text-preview
    SOURCES ${_ut_files} ${_plugin_files}
    LINK_LIBRARIES DFM6::base DFM6::framework Qt6::Widgets KF6::SyntaxHighlighting
)

target_include_directories(ut-text-preview PRIVATE
    "${DFM_PREVIEW_PLUGIN_DIR}"
    ${_preview_dir}
)

