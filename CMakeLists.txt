cmake_minimum_required(VERSION 3.1.0)

project(filemanager)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CMAKE_VERBOSE_MAKEFILEON ON)
    message("debug type open sanitize check")
    set(CMAKE_CXX_FLAGS "-fsanitize=undefined,address,leak -fno-omit-frame-pointer")
    set(CMAKE_C_FLAGS "-fsanitize=undefined,address,leak -fno-omit-frame-pointer")
    set(CMAKE_L_FLAGS "-fsanitize=undefined,address,leak -fno-omit-frame-pointer")
#    set(CMAKE_CXX_FLAGS "-fsanitize=thread")
#    set(CMAKE_C_FLAGS "-fsanitize=thread")
#    set(CMAKE_L_FLAGS "-fsanitize=thread")
endif()

# Setup install paths
#   /usr/bin/dde-file-manager
#       execute bin file
#   /usr/lib/x86_64-linux-gnu/dde-file-manager
#       dir plugins && dir tools abv thumbnail
#   /usr/include/dde-file-manager
#       public *.h && private dir *.h
#   /usr/share/dde-file-manager
#       subdir"database  mimetypeassociations  mimetypes  templates  translations"
set(LIBRARY_INSTALL_PREFIX "/usr/lib/${CMAKE_LIBRARY_ARCHITECTURE}")
#set(ARCHIVE_INSTALL_RPEFIX "") undefined the static lib archive
set(RUNTIME_INSTALL_RPEFIX "/usr/bin")
set(SOURCES_INSTALL_RPEFIX "/usr/share/dde-file-manager")
set(INCLUDE_INSTALL_RPEFIX "/usr/include")

set(DFM_PLUGIN_PATH ${LIBRARY_INSTALL_PREFIX}/plugins/dfm-framework)

#if no debug, can't out in code define key '__FUNCTION__' and so on
add_definitions(-DQT_MESSAGELOGCONTEXT)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/build/${CMAKE_BUILD_TYPE})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/build/${CMAKE_BUILD_TYPE})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/build/${CMAKE_BUILD_TYPE})

# Setup the environment
set(CMAKE_CXX_STANDARD 17)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall -Wl,--as-need -fPIE")

# Use Qt modules
set(QtFindModules Core Gui Widgets Concurrent Svg Network Sql X11Extras Xml DBus)
foreach(QtModule ${QtFindModules})
    find_package(Qt5 COMPONENTS ${QtModule} REQUIRED)
    # include qt module private include directors
    include_directories(${Qt5${QtModule}_PRIVATE_INCLUDE_DIRS})
    # can use target_link_libraries(xxx ${QtUseModules})
    list(APPEND QtUseModules "Qt5::${QtModule}")
    message("QtModule found ${QtModule} OK!")
endforeach()

#other not found libs kde from input for Qt
set(KdeFindPackage KF5Codecs)
foreach(KdeConfigModule ${KdeFindPackage})
    find_package(${KdeConfigModule} REQUIRED)
    if(${KdeConfigModule}_FOUND)
        #can use target_link_libraries(xxx ${QtUseModules})
        list(APPEND QtUseModules "${KdeConfigModule}")
        include_directories(${${KdeConfigModule}_INCLUDE_DIRS})
        include_directories(/usr/include/KF5/KCodecs/)
    else()
        message(FATAL_ERROR "NotFound ${KdeConfigModule}")
    endif(${CMakeConfigModule}_FOUND)
endforeach(KdeConfigModule)

# Use PkgConfig modules
# You can setting mini version for "glib-2.0>=2.10 gtk+-2.0" in the list
set(PkgConfigFindModules dtkcore dtkwidget dtkgui gio-unix-2.0 dfm-io dfm-mount)
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

#define service source dir
set(COMMON_SERVICE_DIR ${PROJECT_SOURCE_DIR}/src/services/common)
set(FILEMANAGER_SERVICE_DIR ${PROJECT_SOURCE_DIR}/src/services/business/filemanager)
set(DESKTOP_SERVICE_DIR ${PROJECT_SOURCE_DIR}/src/services/business/desktop)
#any sub dir can use common service
include_directories(${COMMON_SERVICE_DIR})

# sub directories
add_subdirectory(${PROJECT_SOURCE_DIR}/src/apps)
add_subdirectory(${PROJECT_SOURCE_DIR}/src/dde-dock-plugins)
add_subdirectory(${PROJECT_SOURCE_DIR}/src/dfm-base)
add_subdirectory(${PROJECT_SOURCE_DIR}/src/dfm-extension)
add_subdirectory(${PROJECT_SOURCE_DIR}/src/dfm-framework)
add_subdirectory(${PROJECT_SOURCE_DIR}/src/plugins)
add_subdirectory(${PROJECT_SOURCE_DIR}/src/services)
