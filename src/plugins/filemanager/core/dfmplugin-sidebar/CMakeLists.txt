cmake_minimum_required(VERSION 3.10)

project(dfmplugin-sidebar)

add_definitions(-DSIDEBAR_TREEVIEW)
set(SIDEBAR_TREEVIEW ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

if (SIDEBAR_TREEVIEW)
    add_definitions(-DTREEVIEW)
    message("Sidebar is implemented with tree view")
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/treeviews)
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/treemodels)
    FILE(GLOB SIDEBAR_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/sidebar.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/sidebar.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/utils/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/utils/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/events/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/events/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/treemodels/sidebarmodel.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/treemodels/sidebarmodel.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/treeviews/sidebarview.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/treeviews/sidebarview.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/treeviews/sidebaritem.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/treeviews/sidebaritem.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/treeviews/sidebaritemdelegate.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/treeviews/sidebaritemdelegate.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/treeviews/sidebarwidget.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/treeviews/sidebarwidget.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/treeviews/private/sidebarview_p.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.json"
    )
else()
    message("Sidebar is implemented with list view")
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/views)
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/models)
FILE(GLOB SIDEBAR_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/sidebar.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/sidebar.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/utils/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/utils/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/events/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/events/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/models/sidebarmodel.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/models/sidebarmodel.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/views/sidebarview.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/views/sidebarview.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/views/sidebaritem.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/views/sidebaritem.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/views/sidebaritemdelegate.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/views/sidebaritemdelegate.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/views/sidebarwidget.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/views/sidebarwidget.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/views/private/sidebarview_p.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.json"
    )
endif()
find_package(Dtk REQUIRED Widget REQUIRED)

add_library(${PROJECT_NAME}
    SHARED
    ${SIDEBAR_FILES}
)

set_target_properties(${PROJECT_NAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ../../)

target_include_directories(${PROJECT_NAME}
    PUBLIC
        ${DtkWidget_INCLUDE_DIRS}
)

target_link_libraries(${PROJECT_NAME}
    DFM::base
    DFM::framework
    ${DtkWidget_LIBRARIES}
)

#install library file
install(TARGETS
    ${PROJECT_NAME}
    LIBRARY
    DESTINATION
    ${DFM_PLUGIN_FILEMANAGER_CORE_DIR}
)

# install dconfig files.
include(${CMAKE_SOURCE_DIR}/install_dconfig.cmake)
INSTALL_DCONFIG("org.deepin.dde.file-manager.sidebar.json")

