cmake_minimum_required(VERSION 3.10)

# Unit tests for music-preview: recompile plugin sources into the test binary
# so stub-ext can access private members (see autotests/README.md).
set(_preview_dir "${DFM_PREVIEW_PLUGIN_DIR}/music-preview")

file(GLOB_RECURSE _ut_files
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h")

file(GLOB_RECURSE _plugin_files
    "${_preview_dir}/*.cpp"
    "${_preview_dir}/*.h")

find_package(Qt6 COMPONENTS Widgets REQUIRED)
find_package(Qt6 COMPONENTS Multimedia REQUIRED)
find_package(ICU COMPONENTS i18n io uc REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_search_module(taglib REQUIRED taglib)

dfm_create_test_executable(ut-music-preview
    SOURCES ${_ut_files} ${_plugin_files}
    LINK_LIBRARIES DFM6::base DFM6::framework Qt6::Widgets Qt6::Multimedia ICU::i18n ${taglib_LIBRARIES}
)

target_include_directories(ut-music-preview PRIVATE
    "${DFM_PREVIEW_PLUGIN_DIR}"
    ${_preview_dir}
)
target_include_directories(ut-music-preview PRIVATE ${taglib_INCLUDE_DIRS})
