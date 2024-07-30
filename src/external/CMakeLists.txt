cmake_minimum_required(VERSION 3.10)

add_subdirectory(dde-dock-plugins)

if (OPT_ENABLE_QT6)
    add_subdirectory(dde-shell-plugins)
endif()
