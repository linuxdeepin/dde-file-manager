cmake_minimum_required(VERSION 3.1.0)

project(dfmservice-filemanager)

# window service sources

set(GLOBAL_H
    dfm_filemanager_service_global.h
    )

set(WINDOWSERVICE_CPP
    windowservice/private/browsewindow_p.cpp
    windowservice/windowservice.cpp
    windowservice/displayviewlogic.cpp
    windowservice/displaydefaultview.cpp
    windowservice/navwidget.cpp
    windowservice/addressbar.cpp
    windowservice/completerview.cpp
    windowservice/optionbuttonbox.cpp
    windowservice/crumbbar.cpp
    windowservice/browseview.cpp
    windowservice/browsewindow.cpp
    windowservice/crumbmodel.cpp
    )

set(WINDOWSERVICE_H
    windowservice/private/browsewindow_p.h
    windowservice/private/addressbar_p.h
    windowservice/private/crumbbar_p.h
    windowservice/private/navwidget_p.h
    windowservice/private/optionbuttonbox_p.h
    windowservice/private/completerview_p.h
    windowservice/private/displaydefaultview_p.h
    windowservice/contexts.h
    windowservice/windowservice.h
    windowservice/displayviewlogic.h
    windowservice/displaydefaultview.h
    windowservice/navwidget.h
    windowservice/addressbar.h
    windowservice/completerview.h
    windowservice/optionbuttonbox.h
    windowservice/crumbbar.h
    windowservice/browseview.h
    windowservice/browsewindow.h
    windowservice/crumbmodel.h
    )

set(APPLICATIONSERVICE_CPP
    applicationservice/application.cpp
    applicationservice/settings.cpp
    )

set(APPLICATIONSERVICE_H
    applicationservice/application.h
    applicationservice/private/application_p.h
    applicationservice/settings.h
    )

set(SHORTCUTSERVICE_H
    shortcutservice/shortcut.h)

set(SHORTCUTSERVICE_CPP
    shortcutservice/shortcut.cpp)

add_library(
    ${PROJECT_NAME}
    SHARED
    ${GLOBAL_H}
    ${WINDOWSERVICE_H}
    ${WINDOWSERVICE_CPP}
    ${APPLICATIONSERVICE_H}
    ${APPLICATIONSERVICE_CPP}
    )

target_link_libraries(
    ${PROJECT_NAME}
    dfm-framework
    ${QtUseModules}
    ${PkgUseModules}
    )

