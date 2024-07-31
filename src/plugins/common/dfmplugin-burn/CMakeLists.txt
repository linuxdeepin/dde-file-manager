cmake_minimum_required(VERSION 3.10)

project(dfmplugin-burn)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

FILE(GLOB_RECURSE BURN_FILES CONFIGURE_DEPENDS
    "./*.h"
    "./*.cpp"
    "./*.json"
    )

include(dfmplugin-burn-qt5.cmake)

# Qt6
if (DFM_BUILD_WITH_QT6)
include(dfmplugin-burn-qt6.cmake)
endif()

# install dconfig files.
INSTALL_DCONFIG("org.deepin.dde.file-manager.burn.json")
