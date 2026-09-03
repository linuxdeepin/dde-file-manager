# Plugin unit tests, grouped by application entity.
# Each group is gated by its own OPT_ENABLE_BUILD_UT_* option (default ON).

option(OPT_ENABLE_BUILD_UT_DESKTOP     "Build desktop plugin unit tests"      ON)
option(OPT_ENABLE_BUILD_UT_FILEMANAGER "Build file-manager plugin unit tests" ON)
option(OPT_ENABLE_BUILD_UT_COMMON      "Build common plugin unit tests"       ON)
option(OPT_ENABLE_BUILD_UT_DAEMON      "Build daemon plugin unit tests"       ON)
option(OPT_ENABLE_BUILD_UT_FILEDIALOG  "Build filedialog plugin unit tests"   ON)

# ── 桌面 (Desktop) ──────────────────────────────────────────
if(OPT_ENABLE_BUILD_UT_DESKTOP)
    add_subdirectory(ddplugin-background)
    add_subdirectory(ddplugin-canvas)
    add_subdirectory(ddplugin-core)
    add_subdirectory(ddplugin-organizer)
    add_subdirectory(ddplugin-wallpapersetting)
endif()

# ── 文管 (File Manager) ─────────────────────────────────────
if(OPT_ENABLE_BUILD_UT_FILEMANAGER)
    add_subdirectory(dfmplugin-avfsbrowser)
    add_subdirectory(dfmplugin-computer)
    add_subdirectory(dfmplugin-core)
    add_subdirectory(dfmplugin-detailspace)
    add_subdirectory(dfmplugin-disk-encrypt-entry)
    add_subdirectory(dfmplugin-myshares)
    add_subdirectory(dfmplugin-optical)
    add_subdirectory(dfmplugin-recent)
    add_subdirectory(dfmplugin-search)
    add_subdirectory(dfmplugin-sidebar)
    add_subdirectory(dfmplugin-smbbrowser)
    add_subdirectory(dfmplugin-titlebar)
    add_subdirectory(dfmplugin-trash)
    add_subdirectory(dfmplugin-vault)
    add_subdirectory(dfmplugin-workspace)
endif()

# ── 通用 (Common) ───────────────────────────────────────────
if(OPT_ENABLE_BUILD_UT_COMMON)
    add_subdirectory(dfmplugin-bookmark)
    add_subdirectory(dfmplugin-burn)
    add_subdirectory(dfmplugin-dirshare)
    add_subdirectory(dfmplugin-emblem)
    add_subdirectory(dfmplugin-fileoperations)
    add_subdirectory(dfmplugin-menu)
    add_subdirectory(dfmplugin-propertydialog)
    add_subdirectory(dfmplugin-tag)
    add_subdirectory(dfmplugin-trashcore)
    add_subdirectory(dfmplugin-utils)
endif()

# ── Daemon ──────────────────────────────────────────────────
if(OPT_ENABLE_BUILD_UT_DAEMON)
    add_subdirectory(dfmdaemon-core)
    add_subdirectory(dfmdaemon-filemanager1)
    add_subdirectory(dfmdaemon-recent)
    add_subdirectory(dfmdaemon-tag)
    add_subdirectory(dfmdaemon-vault)
endif()

# ── 文件对话框 (File Dialog) ────────────────────────────────
if(OPT_ENABLE_BUILD_UT_FILEDIALOG)
    add_subdirectory(filedialog-core)
endif()
