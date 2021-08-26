cmake_minimum_required(VERSION 3.1.0)

project(dfm-filemanager-service)

# window service sources
set(windowservice_cpp
    windowservice/dfmwindowservice.cpp
    windowservice/dfmdisplayviewlogic.cpp
    windowservice/dfmdisplaydefaultview.cpp
    windowservice/dfmnavwidget.cpp
    windowservice/dfmaddressbar.cpp
    windowservice/dfmcompleterview.cpp
    windowservice/dfmoptionbuttonbox.cpp
    windowservice/dfmcrumbbar.cpp
    windowservice/private/dfmbrowsewindow_p.cpp
    windowservice/dfmbrowseview.cpp
    windowservice/dfmbrowsewindow.cpp
    windowservice/dfmcrumbmodel.cpp
    )

set(windowservice_h
    windowservice/dfmwindowservice.h
    windowservice/dfmdisplayviewlogic.h
    windowservice/dfmdisplaydefaultview.h
    windowservice/private/dfmbrowsewindow_p.h
    windowservice/dfmnavwidget.h
    windowservice/private/dfmaddressbar_p.h
    windowservice/dfmaddressbar.h
    windowservice/dfmcompleterview.h
    windowservice/dfmoptionbuttonbox.h
    windowservice/private/dfmcrumbbar_p.h
    windowservice/dfmcrumbbar.h
    windowservice/private/dfmbrowseview_p.h
    windowservice/dfmbrowseview.h
    windowservice/dfmbrowsewindow.h
    windowservice/dfmcrumbmodel.h
    )

add_library(
    ${PROJECT_NAME}
    SHARED
    ${windowservice_h}
    ${windowservice_cpp}
    )

target_link_libraries(
    ${PROJECT_NAME}
    ${QtUseModules}
    ${PkgUseModules}
    )

