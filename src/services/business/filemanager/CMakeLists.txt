cmake_minimum_required(VERSION 3.1.0)

project(dfmservice-filemanager)

# window service sources

set(GLOBAL_H
    dfm_filemanager_service_global.h
    )

set(WINDOWSERVICE_CPP
    window/private/browsewindow_p.cpp
    window/windowservice.cpp
    window/displayviewlogic.cpp
    window/displaydefaultview.cpp
    window/navwidget.cpp
    window/addressbar.cpp
    window/completerview.cpp
    window/optionbuttonbox.cpp
    window/crumbbar.cpp
    window/browseview.cpp
    window/browsewindow.cpp
    window/crumbmodel.cpp
    )

set(WINDOWSERVICE_H
    window/private/browsewindow_p.h
    window/private/addressbar_p.h
    window/private/crumbbar_p.h
    window/private/navwidget_p.h
    window/private/optionbuttonbox_p.h
    window/private/completerview_p.h
    window/private/displaydefaultview_p.h
    window/contexts.h
    window/windowservice.h
    window/displayviewlogic.h
    window/displaydefaultview.h
    window/navwidget.h
    window/addressbar.h
    window/completerview.h
    window/optionbuttonbox.h
    window/crumbbar.h
    window/browseview.h
    window/browsewindow.h
    window/crumbmodel.h
    )

set(SHORTCUTSERVICE_H
    shortcut/shortcut.h)

set(SHORTCUTSERVICE_CPP
    shortcut/shortcut.cpp)

add_library(
    ${PROJECT_NAME}
    SHARED
    ${GLOBAL_H}
    ${WINDOWSERVICE_H}
    ${WINDOWSERVICE_CPP}
    ${SHORTCUTSERVICE_H}
    ${SHORTCUTSERVICE_CPP}
    )

target_link_libraries(
    ${PROJECT_NAME}
    dfm-base
    dfm-framework
    ${QtUseModules}
    ${PkgUseModules}
    )

