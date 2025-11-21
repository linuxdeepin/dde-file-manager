
# Common plugins unit tests
add_subdirectory(dfmplugin-utils)
add_subdirectory(dfmplugin-menu)
add_subdirectory(dfmplugin-propertydialog)
add_subdirectory(dfmplugin-tag)
add_subdirectory(dfmplugin-trashcore)
add_subdirectory(dfmplugin-emblem)
add_subdirectory(dfmplugin-fileoperations)
add_subdirectory(dfmplugin-bookmark)
add_subdirectory(dfmplugin-dirshare)
add_subdirectory(dfmplugin-burn)

# FileManager plugins unit tests
add_subdirectory(dfmplugin-vault)
add_subdirectory(dfmplugin-workspace)
add_subdirectory(dfmplugin-trash)
add_subdirectory(dfmplugin-titlebar)
add_subdirectory(dfmplugin-smbbrowser)
add_subdirectory(dfmplugin-sidebar)
add_subdirectory(dfmplugin-recent)
add_subdirectory(dfmplugin-myshares)
add_subdirectory(dfmplugin-optical)
add_subdirectory(dfmplugin-core)
add_subdirectory(dfmplugin-detailspace)
add_subdirectory(dfmplugin-avfsbrowser)
add_subdirectory(dfmplugin-computer)
add_subdirectory(dfmplugin-search)
if(DFM_TEST_ENABLE_DISKENCRYPT)
    add_subdirectory(dfmplugin-disk-encrypt-entry)
endif()

# Filedialog plugins unit tests
add_subdirectory(filedialog-core)

# Daemon plugins unit tests
add_subdirectory(dfmdaemon-core)
add_subdirectory(dfmdaemon-filemanager1)
add_subdirectory(dfmdaemon-tag)
# add_subdirectory(dfmdaemon-vault)
add_subdirectory(dfmdaemon-recent)

# Desktop plugins unit tests
add_subdirectory(ddplugin-core)
add_subdirectory(ddplugin-canvas)
add_subdirectory(ddplugin-background)
add_subdirectory(ddplugin-organizer)
add_subdirectory(ddplugin-wallpapersetting)
