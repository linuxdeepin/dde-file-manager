cmake_minimum_required(VERSION 3.10)

if(NOT OPT_ENABLE_QT6)
add_subdirectory(dde-dock-plugins)
endif()
add_subdirectory(dde-shell-plugins)
