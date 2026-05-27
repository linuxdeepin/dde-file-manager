# dfm-extension unit tests
# Links against DFM::extension shared library — no source recompilation.
# dfm_extension_stub.cpp provides stub implementations for the plugin C API.

file(GLOB_RECURSE TEST_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
)

dfm_add_test(test-dfm-extension
    SOURCES ${TEST_SOURCES}
    LINK_LIBRARIES DFM::extension Qt6::Test
)
