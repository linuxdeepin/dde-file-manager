cmake_minimum_required(VERSION 3.10)

project(dfm-upgrade)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

include(dfm-upgrade.cmake)

# Qt5
include(dfm-upgrade-qt5.cmake)

# Qt6
if (DFM_BUILD_WITH_QT6)
    include(dfm-upgrade-qt6.cmake)
endif()
