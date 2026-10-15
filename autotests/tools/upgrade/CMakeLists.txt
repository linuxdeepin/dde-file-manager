# Upgrade tool tests
# Links the pre-built shared library dfm-upgrade-qt6 (has coverage instrumentation).
# Exercising library functions updates the library's .gcda files.

file(GLOB_RECURSE TEST_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/test_*.cpp"
)
# coverage-batch TUs (test_cov_*.cpp) pull in their own main and the
# UT_UPGRADE_LIB_PATH define; keep them out of the non-sandboxed target above.
list(FILTER TEST_SOURCES EXCLUDE REGEX "test_cov_.*\\.cpp$")

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

# ---------------------------------------------------------------------------
# Coverage batch target: env-isolated tests (HOME/XDG redirected to a temp
# directory in main_cov.cpp) for upgrade units / dialog / interfaces.
# NOTE: sources are named cov_*.cpp (not test_*.cpp) on purpose so the
# ut-dfm-upgrade glob above does not pick them up: that target's main() does
# not sandbox $HOME and lacks the UT_UPGRADE_LIB_PATH definition.
# ---------------------------------------------------------------------------
file(GLOB COV_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/test_cov_*.cpp"
)

dfm_add_test(ut-dfm-upgrade-cov
    SOURCES
        ${COV_SOURCES}
        "${CMAKE_CURRENT_SOURCE_DIR}/main_cov.cpp"
    LINK_LIBRARIES
        dfm-upgrade-qt6
        DFM6::base
        DFM6::framework
        Qt6::Core
        Qt6::Test
        Qt6::Widgets
        Qt6::Sql
)

target_include_directories(ut-dfm-upgrade-cov PRIVATE
    "${DFM_SOURCE_DIR}/tools/upgrade"
)

target_compile_definitions(ut-dfm-upgrade-cov PRIVATE
    UT_UPGRADE_LIB_PATH="$<TARGET_FILE:dfm-upgrade-qt6>"
)
