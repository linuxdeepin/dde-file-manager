cmake_minimum_required(VERSION 3.10)

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
    )

set(widgets_cxx_h
    dfm-base/widgets/abstractbackground.h
    dfm-base/widgets/abstractscreen.h
    dfm-base/widgets/abstractscreenproxy.h
    dfm-base/widgets/screenglobal.h
    )

set(service_cxx_cpp
    background/services/backgroundservice.cpp
    background/services/backgroundfactory.cpp
    wallpaper/wallpaperservice.cpp
    canvas/services/canvasservice.cpp
    screen/screenservice.cpp
)

set(service_cxx_h
    background/services/backgroundservice.h
    background/services/backgroundfactory.h
    wallpaper/wallpaperservice.h
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
    canvas/view/canvasselectionmodel.cpp
    canvas/view/gridcoordinate.cpp
    canvas/view/operator/boxselecter.cpp
    canvas/view/operator/clickselecter.cpp
    canvas/view/operator/viewpainter.cpp
    canvas/view/operator/operstate.cpp
    canvas/view/operator/keyselecter.cpp
    canvas/view/operator/dragdropoper.cpp
    canvas/view/operator/shortcutoper.cpp
    canvas/view/operator/canvasviewmenuproxy.cpp
    canvas/view/operator/fileoperaterproxy.cpp
    canvas/view/operator/viewsettingutil.cpp
    canvas/delegate/canvasitemdelegate.cpp
    canvas/delegate/elidetextlayout.cpp
    canvas/delegate/itemeditor.cpp
    canvas/delegate/tagtextformat.cpp
    canvas/delegate/filetagobjectinterface.cpp
    canvas/grid/canvasgrid.cpp
    canvas/grid/gridcore.cpp
    canvas/filetreater.cpp
    canvas/displayconfig.cpp
    canvas/menu/canvasmenu.cpp
    watermask/watermaskframe.cpp
    watermask/deepinlicensehelper.cpp
    wallpaper/wallpapersettings.cpp
    wallpaper/wallpaperlist.cpp
    wallpaper/wallpaperitem.cpp
    wallpaper/loadinglabel.cpp
    wallpaper/thumbnailmanager.cpp
    wallpaper/wallaperpreview.cpp
    wallpaper/private/autoactivatewindow.cpp
    wallpaper/dbus/screensaver_interface.cpp
    utils/desktoputils.cpp
    utils/fileutil.cpp
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
    canvas/private/filetreater_p.h
    canvas//canvasmanager.h
    canvas/view/canvasview_p.h
    canvas/view/canvasview.h
    canvas/view/canvasmodel.h
    canvas/view/gridcoordinate.h
    canvas/view/canvasselectionmodel.h
    canvas/view/operator/boxselecter.h
    canvas/view/operator/clickselecter.h
    canvas/view/operator/viewpainter.h
    canvas/view/operator/operstate.h
    canvas/view/operator/keyselecter.h
    canvas/view/operator/dragdropoper.h
    canvas/view/operator/shortcutoper.h
    canvas/view/operator/canvasviewmenuproxy.h
    canvas/view/operator/fileoperaterproxy_p.h
    canvas/view/operator/fileoperaterproxy.h
    canvas/view/operator/viewsettingutil.h
    canvas/delegate/canvasitemdelegate_p.h
    canvas/delegate/canvasitemdelegate.h
    canvas/delegate/elidetextlayout.h
    canvas/delegate/itemeditor.h
    canvas/delegate/tagtextformat.h
    canvas/delegate/filetagobjectinterface.h
    canvas/grid/canvasgrid.h
    canvas/grid/canvasgrid_p.h
    canvas/grid/canvasgridspecialist.h
    canvas/grid/gridcore.h
    canvas/filetreater.h
    canvas/displayconfig.h
    canvas/menu/canvasmenu.h
    watermask/watermaskframe.h
    watermask/deepinlicensehelper.h
    wallpaper/wallpapersettings.h
    wallpaper/private/wallpapersettings_p.h
    wallpaper/private/autoactivatewindow.h
    wallpaper/private/autoactivatewindow_p.h
    wallpaper/wallpaperlist.h
    wallpaper/loadinglabel.h
    wallpaper/wallpaperitem.h
    wallpaper/thumbnailmanager.h
    wallpaper/wallaperpreview.h
    wallpaper/dbus/screensaver_interface.h
    utils/desktoputils.h
    utils/fileutil.h
    screen/screenqt.h
    screen/screenproxyqt.h
    screen/screendbus.h
    screen/screenproxydbus.h
    core.h
    dfm_desktop_service_global.h
)

# 指定资源文件
set(QRCS
    wallpaper/images.qrc
)

add_library(${PROJECT_NAME}
    SHARED
    ${core_cxx_cpp}
    ${core_cxx_h}
    ${service_cxx_cpp}
    ${service_cxx_h}
    ${widgets_cxx_cpp}
    ${widgets_cxx_h}
    ${QRCS}
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
