cmake_minimum_required(VERSION 3.0.2)

project(ddplugin-core)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/background
    ${CMAKE_CURRENT_SOURCE_DIR}/background/services
    ${CMAKE_CURRENT_SOURCE_DIR}/wallpaper
    ${CMAKE_CURRENT_SOURCE_DIR}/canvas
    ${CMAKE_CURRENT_SOURCE_DIR}/canvas/services
    ${CMAKE_CURRENT_SOURCE_DIR}/screen
)

set(widgets_cxx_cpp
    dfm-base/widgets/abstractbackground.cpp
    dfm-base/widgets/abstractscreen.cpp
    dfm-base/widgets/abstractscreenproxy.cpp
    dfm-base/widgets/abstractfiletreater.cpp
    )

set(widgets_cxx_h
    dfm-base/widgets/abstractbackground.h
    dfm-base/widgets/abstractscreen.h
    dfm-base/widgets/abstractscreenproxy.h
    dfm-base/widgets/screenglobal.h
    dfm-base/widgets/abstractfiletreater.h
    )

set(service_cxx_cpp
    background/services/backgroundservice.cpp
    background/services/backgroundfactory.cpp
    wallpaper/wallpaperservice.cpp
    wallpaper/wallpaperfactory.cpp
    canvas/services/canvasservice.cpp
    screen/screenservice.cpp
)

set(service_cxx_h
    background/services/backgroundservice.h
    background/services/backgroundfactory.h
    wallpaper/wallpaperservice.h
    wallpaper/wallpaperfactory.h
    canvas/services/canvasservice.h
    screen/screenservice.h
)

set(core_cxx_cpp
    dbus-private/dbusdisplay.cpp
    dbus-private/dbusdock.cpp
    dbus-private/dbusmonitor.cpp
    dbus-private/licenceInterface.cpp
    background/backgroundmanager.cpp
    background/backgrounddefault.cpp
    canvas/canvasmanager.cpp
    canvas/view/canvasview.cpp
    canvas/view/canvasmodel.cpp
    canvas/view/canvasitemdelegate.cpp
    canvas/view/gridcoordinate.cpp
    canvas/view/operator/boxselecter.cpp
    canvas/canvasgridmanager.cpp
    canvas/grid/canvasgrid.cpp
    canvas/grid/gridcore.cpp
    canvas/defaultdesktopfileinfo.cpp
    canvas/filetreater.cpp
    canvas/displayconfig.cpp
    watermask/watermaskframe.cpp
    watermask/deepinlicensehelper.cpp
    utils/desktoputils.cpp
    screen/screenqt.cpp
    screen/screenproxyqt.cpp
    screen/screendbus.cpp
    screen/screenproxydbus.cpp
    core.cpp
    core.json
)

set(core_cxx_h
    dbus-private/dbusdisplay.h
    dbus-private/dbusdock.h
    dbus-private/dbusmonitor.h
    dbus-private/licenceInterface.h
    background/backgroundmanager.h
    background/backgroundmanager_p.h
    background/backgrounddefault.h
    canvas/private/canvasmanager_p.h
    canvas/private/canvasgridmanager_p.h
    canvas/private/defaultdesktopfileinfo_p.h
    canvas/private/filetreater_p.h
    canvas//canvasmanager.h
    canvas/view/canvasview_p.h
    canvas/view/canvasview.h
    canvas/view/canvasmodel.h
    canvas/view/canvasitemdelegate.h
    canvas/view/canvasitemdelegate_p.h
    canvas/view/gridcoordinate.h
    canvas/view/operator/boxselecter.h
    canvas/canvasgridmanager.h
    canvas/grid/canvasgrid.h
    canvas/grid/canvasgrid_p.h
    canvas/grid/canvasgridspecialist.h
    canvas/grid/gridcore.h
    canvas/defaultdesktopfileinfo.h
    canvas/filetreater.h
    canvas/displayconfig.h
    watermask/watermaskframe.h
    watermask/deepinlicensehelper.h
    utils/desktoputils.h
    screen/screenqt.h
    screen/screenproxyqt.h
    screen/screendbus.h
    screen/screenproxydbus.h
    core.h
    dfm_desktop_service_global.h
)

add_library(${PROJECT_NAME}
    SHARED
    ${core_cxx_cpp}
    ${core_cxx_h}
    ${service_cxx_cpp}
    ${service_cxx_h}
    ${widgets_cxx_cpp}
    ${widgets_cxx_h}
)

set_target_properties(${PROJECT_NAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ../../)

find_package(Dtk REQUIRED Widget REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(DFrameworkDBus REQUIRED dframeworkdbus )
pkg_check_modules(DtkGui REQUIRED dtkgui )
pkg_check_modules(DtkWidget REQUIRED dtkwidget )

target_include_directories(${PROJECT_NAME} PUBLIC
    ${DFrameworkDBus_INCLUDE_DIRS}
    ${GIO-QT_INCLUDE_DIRS}
    ${DtkWidget_INCLUDE_DIRS}
    ${DtkGui_INCLUDE_DIRS}
)

target_link_libraries(${PROJECT_NAME}
    DFM::base
    DFM::framework
    DFMSERVICE::common
    ${DFrameworkDBus_LIBRARIES}
    ${DtkWidget_LIBRARIES}
    ${DtkGui_LIBRARIES}
    )

#install library file
install(TARGETS
    ${PROJECT_NAME}
    LIBRARY
    DESTINATION
    ${DFM_PLUGIN_DIR}
)
