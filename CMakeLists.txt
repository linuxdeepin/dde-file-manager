cmake_minimum_required(VERSION 3.1.0)

project(filemanager)

# Setup the environment
SET(CMAKE_CXX_STANDARD 17)
if(CMAKE_VERSION VERSION_LESS "3.7.0")
    set(CMAKE_INCLUDE_CURRENT_DIR ON)
endif()
SET(CMAKE_AUTOMOC ON)
SET(CMAKE_INCLUDE_CURRENT_DIR ON)
SET(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall -Wl,--as-need -fPIE")


# Use Qt modules
set(QtFindModules Core Gui Widgets Concurrent Svg Network Sql X11Extras)

foreach(QtModule ${QtFindModules})
    find_package(Qt5 COMPONENTS ${QtModule} REQUIRED)

    # include qt module private include directors
    include_directories(${Qt5${QtModule}_PRIVATE_INCLUDE_DIRS})

    # can use target_link_libraries(xxx ${QtUseModules})
    list(APPEND QtUseModules "Qt5::${QtModule}")
    message("QtModule found ${QtModule} OK!")
endforeach()

# Use PkgConfig modules
# You can setting mini version for "glib-2.0>=2.10 gtk+-2.0" in the list
set(PkgConfigFindModules dtkcore dtkwidget dtkgui gio-unix-2.0)

include(FindPkgConfig)

foreach(PkgConfigModule ${PkgConfigFindModules})
    pkg_check_modules(PkgUseModuleVal REQUIRED ${PkgConfigModule})
    #can use target_link_libraries(xxx ${PkgUseModule})
    list(APPEND PkgUseModules ${PkgUseModuleVal_LDFLAGS})
    include_directories(${PkgUseModuleVal_INCLUDE_DIRS})
    #    link_libraries(${PkgUseModuleVal_LIBRARIES})
    #    link_directories(${PkgUseModuleVal_LINK_LIBRARIES})
endforeach(PkgConfigModule)

include_directories(${PROJECT_SOURCE_DIR}/src)
# sub directories
add_subdirectory(${PROJECT_SOURCE_DIR}/src/apps)
add_subdirectory(${PROJECT_SOURCE_DIR}/src/dde-dock-plugins)
add_subdirectory(${PROJECT_SOURCE_DIR}/src/dfm-base)
add_subdirectory(${PROJECT_SOURCE_DIR}/src/dfm-extension)
add_subdirectory(${PROJECT_SOURCE_DIR}/src/dfm-framework)
add_subdirectory(${PROJECT_SOURCE_DIR}/src/plugins)
add_subdirectory(${PROJECT_SOURCE_DIR}/src/services)
