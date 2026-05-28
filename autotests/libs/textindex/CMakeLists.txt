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

dfm_add_test(test-textindex
    SOURCES ${TEST_SOURCES}
    LINK_LIBRARIES dde-filemanager-textindex dfm6-search Qt6::Core Qt6::Test PkgConfig::NL
)

# textindex source files use relative includes (e.g. "service_textindex_global.h")
# so the textindex source root must be in the include path.
target_include_directories(test-textindex PRIVATE ${DFM_SOURCE_DIR}/services/textindex)
