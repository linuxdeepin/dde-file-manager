cmake_minimum_required(VERSION 3.10)

project(dfmplugin-bookmark)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

FILE(GLOB_RECURSE BOOKMARK_FILES CONFIGURE_DEPENDS
    "./*.h"
    "./*.cpp"
    "./*.json"
)

include(dfmplugin-bookmark-qt5.cmake)

# Qt6
if (DFM_BUILD_WITH_QT6)
include(dfmplugin-bookmark-qt6.cmake)
endif()

