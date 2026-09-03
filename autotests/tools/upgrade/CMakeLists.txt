# Upgrade tool tests
# Links the pre-built shared library dfm-upgrade-qt6 (has coverage instrumentation).
# Exercising library functions updates the library's .gcda files.

file(GLOB_RECURSE TEST_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/test_*.cpp"
)

dfm_add_test(ut-dfm-upgrade
    SOURCES
        ${TEST_SOURCES}
    LINK_LIBRARIES
        dfm-upgrade-qt6
        DFM6::base
        DFM6::framework
        Qt6::Core
        Qt6::Test
)

target_include_directories(ut-dfm-upgrade PRIVATE
    "${DFM_SOURCE_DIR}/tools/upgrade"
)
