cmake_minimum_required(VERSION 3.1.0)

project(dfm-base)

set(GLOBAL_H
    dfm_base_global.h)

set(UTILS_H
    utils/threadcontainer.hpp
    )

set(UTILS_CPP
    )


set(SHUTIL_H
    shutil/fileutils.h
    shutil/mimedatabase.h
    )

set(SHUTIL_CPP
    shutil/fileutils.cpp
    shutil/mimedatabase.cpp
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
    )

set(LOCALFILE_H
    localfile/private/localmenu_p.h
    localfile/private/localdiriterator_p.h
    localfile/private/localfileinfo_p.h
    localfile/private/localfilewatcher_p.h
    localfile/localdiriterator.h
    localfile/localfileinfo.h
    localfile/localmenu.h
    localfile/localfilewatcher.h
    )

set(LOCALFILE_CPP
    localfile/localfileinfo.cpp
    localfile/localmenu.cpp
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
    )

set(QRCS
    resources/resources.qrc
    )

add_library(${PROJECT_NAME}
    SHARED
    ${GLOBAL_H}
    ${BASE_H}
    ${SHUTIL_H}
    ${UTILS_H}
    ${WIDGETS_H}
    ${LOCALFILE_H}
    ${BASE_CPP}
    ${SHUTIL_CPP}
    ${UTILS_CPP}
    ${WIDGETS_CPP}
    ${LOCALFILE_CPP}
    ${QRCS}
    )

target_link_libraries(
    ${PROJECT_NAME}
    ${QtUseModules}
    ${PkgUseModules}
    )
