# First-tier plugin unit tests (high value, low API drift).
# These recompile each plugin's own sources into the test binary so that
# stub-ext can access private members. See autotests/README.md.
add_subdirectory(dfmplugin-fileoperations)
add_subdirectory(ddplugin-organizer)
add_subdirectory(dfmplugin-workspace)
add_subdirectory(dfmplugin-titlebar)
add_subdirectory(dfmplugin-menu)
add_subdirectory(dfmplugin-myshares)
