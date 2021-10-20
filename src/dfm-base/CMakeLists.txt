cmake_minimum_required(VERSION 3.1.0)

project(dfm-base)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/dbusservice)

set(GLOBAL_H
    dfm_base_global.h)

set(UTILS_H
    utils/threadcontainer.hpp
    utils/fileutils.h
    utils/iconutils.h
    utils/universalutils.h
    )

set(UTILS_CPP
    utils/fileutils.cpp
    utils/universalutils.cpp
    )


set(MIMETYPE_H
    mimetype/mimedatabase.h
    )

set(MIMETYPE_H
    mimetype/mimedatabase.cpp
    )

set(BASE_H
    base/private/abstractfilewatcher_p.h
    base/private/abstractfiledevice_p.h
    base/private/abstractfileinfo_p.h
    base/private/subscriber.h
    base/private/infocache.h
    base/private/infocache_p.h
    base/private/watchercache.h
    base/private/watchercache_p.h
    base/abstractfileinfo.h
    base/standardpaths.h
    base/singleton.h
    base/urlroute.h
    base/schemefactory.h
    base/abstractdiriterator.h
    base/schemefactory.h
    base/abstractfilewatcher.h
    base/abstractfiledevice.h
    base/abstractfilemenu.h
    )

set(BASE_CPP
    base/private/subscriber.cpp
    base/private/infocache.cpp
    base/private/watchercache.cpp
    base/abstractfileinfo.cpp
    base/urlroute.cpp
    base/standardpaths.cpp
    base/schemefactory.cpp
    base/abstractfilewatcher.cpp
    base/abstractfiledevice.cpp
    base/abstractfilemenu.cpp
    )

set(LOCALFILE_H
    localfile/private/localdiriterator_p.h
    localfile/private/localfileinfo_p.h
    localfile/private/localfilewatcher_p.h
    localfile/localdiriterator.h
    localfile/localfileinfo.h
    localfile/localfilemenu.h
    localfile/localfilewatcher.h
    )

set(LOCALFILE_CPP
    localfile/localfileinfo.cpp
    localfile/localfilemenu.cpp
    localfile/localdiriterator.cpp
    localfile/localfilewatcher.cpp
    )

set(WIDGETS_H
    widgets/dfmfileview/private/fileview_p.h
    widgets/dfmfileview/private/fileviewmodel_p.h
    widgets/dfmfileview/private/traversaldirthread.h
    widgets/dfmfileview/private/fileviewitem_p.h
    widgets/dfmfileview/private/delegatecommon.h
    widgets/dfmfileview/private/iconitemdelegate_p.h
    widgets/dfmfileview/private/listitemdelegate_p.h
    widgets/dfmsidebar/private/sidebar_p.h
    widgets/dfmsidebar/private/sidebarview_p.h
    widgets/dfmsidebar/private/sidebarmodel_p.h
    widgets/dfmsidebar/sidebar.h
    widgets/dfmsidebar/sidebaritem.h
    widgets/dfmsidebar/sidebaritemdelegate.h
    widgets/dfmsidebar/sidebarmodel.h
    widgets/dfmsidebar/sidebarview.h
    widgets/dfmsplitter/splitter.h
    widgets/dfmfileview/fileview.h
    widgets/dfmfileview/fileviewmodel.h
    widgets/dfmfileview/headerview.h
    widgets/dfmfileview/fileviewitem.h
    widgets/dfmfileview/listitemdelegate.h
    widgets/dfmfileview/iconitemdelegate.h
    widgets/dfmfileview/itemdelegatehelper.h
    widgets/dfmdialog/errordialog.h
    widgets/dfmdialog/contextdialog.h
    )

set(WIDGETS_CPP
    widgets/dfmfileview/private/traversaldirthread.cpp
    widgets/dfmfileview/private/fileview_p.cpp
    widgets/dfmfileview/private/delegatecommon.cpp
    widgets/dfmfileview/private/iconitemdelegate_p.cpp
    widgets/dfmfileview/private/listitemdelegate_p.cpp
    widgets/dfmsidebar/private/sidebar_p.cpp
    widgets/dfmsidebar/sidebar.cpp
    widgets/dfmsidebar/sidebaritem.cpp
    widgets/dfmsidebar/sidebaritemdelegate.cpp
    widgets/dfmsidebar/sidebarmodel.cpp
    widgets/dfmsidebar/sidebarview.cpp
    widgets/dfmsplitter/splitter.cpp
    widgets/dfmfileview/fileview.cpp
    widgets/dfmfileview/fileviewmodel.cpp
    widgets/dfmfileview/headerview.cpp
    widgets/dfmfileview/fileviewitem.cpp
    widgets/dfmfileview/listitemdelegate.cpp
    widgets/dfmfileview/iconitemdelegate.cpp
    widgets/dfmfileview/itemdelegatehelper.cpp
    widgets/dfmdialog/errordialog.cpp
    widgets/dfmdialog/contextdialog.cpp
    )

set(APPLICATIONSERVICE_CPP
    application/application.cpp
    application/settings.cpp
    )

set(APPLICATIONSERVICE_H
    application/application.h
    application/private/application_p.h
    application/settings.h
    )

set(DBUSSERVICE_H
    dbusservice/dbus_interface/devicemanagerdbus_interface.h
    )

set(DBUSSERVICE_CPP
    dbusservice/dbus_interface/devicemanagerdbus_interface.cpp
    )

add_library(${PROJECT_NAME}
    SHARED
    ${GLOBAL_H}
    ${BASE_H}
    ${BASE_CPP}
    ${LOCALFILE_H}
    ${LOCALFILE_CPP}
    ${MIMETYPE_H}
    ${MIMETYPE_CPP}
    ${UTILS_H}
    ${UTILS_CPP}
    ${WIDGETS_H}
    ${WIDGETS_CPP}
    ${APPLICATIONSERVICE_H}
    ${APPLICATIONSERVICE_CPP}
    ${DBUSSERVICE_H}
    ${DBUSSERVICE_CPP}
    ${QRCS}
    )

target_link_libraries(
    ${PROJECT_NAME}
    ${QtUseModules}
    ${PkgUseModules}
    )

add_library(DFM::base ALIAS ${PROJECT_NAME})


