cmake_minimum_required(VERSION 3.10)

add_subdirectory(dde-dock-plugins)

if (DFM_BUILD_WITH_QT6)
    add_subdirectory(dde-shell-plugins)
endif()
