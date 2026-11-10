cmake_minimum_required(VERSION 3.10)

add_subdirectory(panel-desktop)

# The folder-content plugin needs the updated dock API shipped in
# dde-shell 2.0.62 (dock/foldercontent.h, Dde::ShellDock). Build it only when
# the plugin sources are present AND a new enough dde-shell is installed;
# otherwise skip it instead of failing the whole build.
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/folder-content/CMakeLists.txt")
    if(NOT TARGET Dde::ShellDock)
        find_package(DDEShellDock 2.0.62 QUIET)
    endif()
    if(TARGET Dde::ShellDock)
        add_subdirectory(folder-content)
    else()
        message(STATUS "dde-shell >= 2.0.62 not found, skipping folder-content plugin")
    endif()
endif()
