cmake_minimum_required(VERSION 3.7)

project(filemanager)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall -Wl,--as-need -fPIE")


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


#if no debug, can't out in code define key '__FUNCTION__' and so on
add_definitions(-DQT_MESSAGELOGCONTEXT)

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
set(PkgConfigFindModules dtkcore dtkwidget dtkgui gio-unix-2.0 dfm-io dfm-mount gsettings-qt)
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
set(FILEMANAGER_SERVICE_DIR ${PROJECT_SOURCE_DIR}/src/services/filemanager)
set(DESKTOP_SERVICE_DIR ${PROJECT_SOURCE_DIR}/src/services/desktop)
#any sub dir can use common service
include_directories(${COMMON_SERVICE_DIR})

# Install settings

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX /usr)
endif()

include(GNUInstallDirs)
if(NOT DEFINED LIB_INSTALL_DIR)
    set(LIB_INSTALL_DIR   ${CMAKE_INSTALL_FULL_LIBDIR})
endif()

if(NOT DEFINED DFM_PLUGIN_DIR)
    set(DFM_PLUGIN_DIR ${LIB_INSTALL_DIR}/dde-file-manager/plugins)
endif()

# sub directories
add_subdirectory(src/apps)
add_subdirectory(src/dde-dock-plugins)
add_subdirectory(src/dfm-base)
add_subdirectory(src/dfm-extension)
add_subdirectory(src/dfm-framework)
add_subdirectory(src/plugins)
add_subdirectory(src/services)

option(ENABLE_UNIT_TESTS "Enable unit tests" ON)
message(STATUS "Enable testing: ${ENABLE_UNIT_TESTS}")
if(ENABLE_UNIT_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()