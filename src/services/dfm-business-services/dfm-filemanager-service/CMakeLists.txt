cmake_minimum_required(VERSION 3.1.0)

project(dfm-filemanager-service)

# window service sources

set(${PROJECT_NAME}_global_h
    dfm_filemanager_service_global.h
    )

set(windowservice_cpp
    windowservice/windowservice.cpp
    windowservice/displayviewlogic.cpp
    windowservice/displaydefaultview.cpp
    windowservice/navwidget.cpp
    windowservice/addressbar.cpp
    windowservice/completerview.cpp
    windowservice/optionbuttonbox.cpp
    windowservice/crumbbar.cpp
    windowservice/private/browsewindow_p.cpp
    windowservice/browseview.cpp
    windowservice/browsewindow.cpp
    windowservice/crumbmodel.cpp
    )

set(windowservice_h
    windowservice/containerkeys.h
    windowservice/windowservice.h
    windowservice/displayviewlogic.h
    windowservice/displaydefaultview.h
    windowservice/private/browsewindow_p.h
    windowservice/navwidget.h
    windowservice/private/addressbar_p.h
    windowservice/addressbar.h
    windowservice/completerview.h
    windowservice/optionbuttonbox.h
    windowservice/private/crumbbar_p.h
    windowservice/crumbbar.h
    windowservice/private/browseview_p.h
    windowservice/browseview.h
    windowservice/browsewindow.h
    windowservice/crumbmodel.h
    )

set(applicationservice_cpp
    applicationservice/application.cpp
    applicationservice/settings.cpp
    )

set(applicationservice_h
    applicationservice/application.h
    applicationservice/private/application_p.h
    applicationservice/settings.h
    )

add_library(
    ${PROJECT_NAME}
    SHARED
    ${${PROJECT_NAME}_global_h}
    ${windowservice_h}
    ${windowservice_cpp}
    ${applicationservice_h}
    ${applicationservice_cpp}
    )

target_link_libraries(
    ${PROJECT_NAME}
    ${QtUseModules}
    ${PkgUseModules}
    )

