cmake_minimum_required(VERSION 3.0.2)

project(ddplugin-core)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/background
    ${CMAKE_CURRENT_SOURCE_DIR}/background/services
    ${CMAKE_CURRENT_SOURCE_DIR}/wallpaper
    ${CMAKE_CURRENT_SOURCE_DIR}/canvas
    ${CMAKE_CURRENT_SOURCE_DIR}/screen
)

set(widgets_cxx_cpp
    dfm-base/widgets/abstractcanvas.cpp
    dfm-base/widgets/abstractcanvasmodel.cpp
    dfm-base/widgets/abstractcanvasdelegate.cpp
    dfm-base/widgets/abstractcanvasmodelwatcher.cpp
    dfm-base/widgets/abstractcanvasmodelgrid.cpp
    dfm-base/widgets/abstractcanvasselectionmodel.cpp
    dfm-base/widgets/abstractwallpaperitem.cpp
    dfm-base/widgets/abstractwallpaperlist.cpp
    dfm-base/widgets/abstractwallpapersetting.cpp
    dfm-base/widgets/abstractbackgroundmanager.cpp
    dfm-base/widgets/abstractbackground.cpp
    dfm-base/widgets/abstractscreen.cpp
    dfm-base/widgets/abstractscreenproxy.cpp
)

set(widgets_cxx_h
    dfm-base/widgets/abstractcanvas.h
    dfm-base/widgets/abstractcanvasmodel.h
    dfm-base/widgets/abstractcanvasdelegate.h
    dfm-base/widgets/abstractcanvasmodelwatcher.h
    dfm-base/widgets/abstractcanvasmodelgrid.h
    dfm-base/widgets/abstractcanvasselectionmodel.h
    dfm-base/widgets/abstractwallpaperitem.h
    dfm-base/widgets/abstractwallpaperlist.h
    dfm-base/widgets/abstractwallpapersetting.h
    dfm-base/widgets/abstractbackgroundmanager.h
    dfm-base/widgets/abstractbackgroundmanager.h
    dfm-base/widgets/abstractbackground.h
    dfm-base/widgets/abstractscreen.h
    dfm-base/widgets/abstractscreenproxy.h
    dfm-base/widgets/screenglobal.h
)

set(service_cxx_cpp
    background/services/backgroundservice.cpp
    background/services/backgroundfactory.cpp
    wallpaper/wallpaperservice.cpp
    wallpaper/wallpaperfactory.cpp
    canvas/canvasservice.cpp
    canvas/canvasfactory.cpp
    screen/screenservice.cpp
)

set(service_cxx_h
    background/services/backgroundservice.h
    background/services/backgroundfactory.h
    wallpaper/wallpaperservice.h
    wallpaper/wallpaperfactory.h
    canvas/canvasservice.h
    canvas/canvasfactory.h
    screen/screenservice.h
)

set(core_cxx_cpp
    dbus-private/dbusdisplay.cpp
    dbus-private/dbusdock.cpp
    dbus-private/dbusmonitor.cpp
    dbus-private/licenceInterface.cpp
    background/backgroundmanager.cpp
    background/backgrounddefault.cpp
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
pkg_check_modules(GIO-QT REQUIRED gio-qt)

target_include_directories(${PROJECT_NAME} 
    PUBLIC
        ${DFrameworkDBus_INCLUDE_DIRS}
        ${GIO-QT_INCLUDE_DIRS}
        ${DtkWidget_INCLUDE_DIRS}
)

target_link_libraries(${PROJECT_NAME}
    DFM::base
    DFM::framework
    DFMSERVICE::common
    ${DFrameworkDBus_LIBRARIES}
    ${GIO-QT_LIBRARIES}
    ${DtkWidget_LIBRARIES}
)

#install library file
install(TARGETS
    ${PROJECT_NAME}
    LIBRARY
    DESTINATION
    ${DFM_PLUGIN_DIR}
)
