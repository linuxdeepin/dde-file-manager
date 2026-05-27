# dfm-framework unit tests
# Links against DFM6::framework shared library — no source recompilation.

file(GLOB_RECURSE TEST_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
)

dfm_add_test(test-dfm-framework
    SOURCES ${TEST_SOURCES}
    LINK_LIBRARIES DFM6::framework Qt6::Test
)
