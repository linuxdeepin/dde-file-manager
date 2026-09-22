# textindex service unit tests
# Tests link against dde-filemanager-textindex shared library.

find_package(Qt6 REQUIRED COMPONENTS Core Test)
find_package(dfm6-search REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(NL REQUIRED IMPORTED_TARGET libnl-3.0 libnl-genl-3.0)

file(GLOB_RECURSE TEST_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
)

# The system-installed dfm6-search/field_names.h only defines kFileExt under
# the FileName namespace. The textindex source files reference
# Content::kFileExt and OcrText::kFileExt, causing a compilation failure.
# Force-include a test-only compatibility header into the library build so
# those symbols resolve without modifying production source files.
if(TARGET dde-filemanager-textindex)
    target_compile_options(dde-filemanager-textindex PRIVATE
        -include "${CMAKE_CURRENT_SOURCE_DIR}/field_names_compat.h"
    )
endif()

dfm_add_test(ut-textindex
    SOURCES ${TEST_SOURCES}
    LINK_LIBRARIES dde-filemanager-textindex dde-file-manager-extractor-lib dfm6-search Qt6::Core Qt6::Test PkgConfig::NL
)

# textindex source files use relative includes (e.g. "service_textindex_global.h")
# so the textindex source root must be in the include path.
target_include_directories(ut-textindex PRIVATE
    ${DFM_SOURCE_DIR}/services/textindex
    ${DFM_PROJECT_ROOT}/include
    ${DFM_SOURCE_DIR}/apps/dde-file-manager-extractor/libextractor
    ${CMAKE_BINARY_DIR}/src/services/textindex
)
