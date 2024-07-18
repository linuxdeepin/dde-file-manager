cmake_minimum_required(VERSION 3.10)

# add sub dir for business plugins
add_subdirectory(core/dfmplugin-fileoperations)
add_subdirectory(core/dfmplugin-propertydialog)
add_subdirectory(core/dfmplugin-trashcore) # TODO(zhangs): merge it
add_subdirectory(core/dfmplugin-menu)
if(NOT OPT_ENABLE_QT6)
    add_subdirectory(core/dfmplugin-bookmark)
    add_subdirectory(dfmplugin-burn)
    add_subdirectory(dfmplugin-preview)
    add_subdirectory(dfmplugin-emblem)
    add_subdirectory(dfmplugin-dirshare)
    add_subdirectory(dfmplugin-tag)
endif()
add_subdirectory(dfmplugin-utils)

