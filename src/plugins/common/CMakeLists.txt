cmake_minimum_required(VERSION 3.10)

# add sub dir for business plugins
add_subdirectory(dfmplugin-fileoperations)
add_subdirectory(dfmplugin-propertydialog)
add_subdirectory(dfmplugin-trashcore) # TODO(zhangs): merge it
add_subdirectory(dfmplugin-menu)
add_subdirectory(dfmplugin-bookmark)
add_subdirectory(dfmplugin-burn)
add_subdirectory(dfmplugin-emblem)
add_subdirectory(dfmplugin-dirshare)
add_subdirectory(dfmplugin-tag)
add_subdirectory(dfmplugin-utils)
