cmake_minimum_required(VERSION 3.1.0)

project(dfm-base)

set(${PROJECT_NAME}_global_h
    dfm_base_global.h)

set(${PROJECT_NAME}_utils_h
    utils/threadcontainer.hpp
    )

set(${PROJECT_NAME}_utils_cpp
    )


set(${PROJECT_NAME}_shutil_h
    shutil/fileutils.h
    shutil/mimedatabase.h
    )

set(${PROJECT_NAME}_shutil_cpp
    shutil/fileutils.cpp
    shutil/mimedatabase.cpp
    )

set(${PROJECT_NAME}_base_h
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
    base/singleton.hpp
    base/urlroute.h
    base/schemefactory.h
    base/abstractdiriterator.h
    base/schemefactory.h
    base/abstractfilewatcher.h
    base/abstractfiledevice.h
    base/sort.h
    )

set(${PROJECT_NAME}_base_cpp
    base/private/subscriber.cpp
    base/private/infocache.cpp
    base/private/watchercache.cpp
    base/abstractfileinfo.cpp
    base/urlroute.cpp
    base/standardpaths.cpp
    base/sort.cpp
    base/schemefactory.cpp
    base/abstractfilewatcher.cpp
    base/abstractfiledevice.cpp
    )


set(${PROJECT_NAME}_widgets_cpp
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
    widgets/dfmfileview/private/traversaldirthread.cpp
    widgets/dfmfileview/private/fileview_p.cpp
    widgets/dfmfileview/listitemdelegate.cpp
    widgets/dfmfileview/styleditemdelegate.cpp
    widgets/dfmfileview/iconitemdelegate.cpp
    )

set(${PROJECT_NAME}_localfile_h
    localfile/private/localmenu_p.h
    localfile/private/localdiriterator_p.h
    localfile/private/localfileinfo_p.h
    localfile/localdiriterator.h
    localfile/localfileinfo.h
    localfile/localmenu.h
    )

set(${PROJECT_NAME}_localfile_cpp
    localfile/localfileinfo.cpp
    localfile/localmenu.cpp
    localfile/localdiriterator.cpp
    )

set(${PROJECT_NAME}_widgets_h
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
    widgets/dfmfileview/private/fileview_p.h
    widgets/dfmfileview/private/fileviewmodel_p.h
    widgets/dfmfileview/private/traversaldirthread.h
    widgets/dfmfileview/listitemdelegate.h
    widgets/dfmfileview/styleditemdelegate.h
    widgets/dfmfileview/private/styleditemdelegate_p.h
    widgets/dfmfileview/iconitemdelegate.h
    )

set(${PROJECT_NAME}_qrcs
    resources/resources.qrc
    )

add_library(${PROJECT_NAME}
    SHARED
    ${${PROJECT_NAME}_global_h}
    ${${PROJECT_NAME}_base_h}
    ${${PROJECT_NAME}_shutil_h}
    ${${PROJECT_NAME}_utils_h}
    ${${PROJECT_NAME}_widgets_h}
    ${${PROJECT_NAME}_localfile_h}
    ${${PROJECT_NAME}_job_h}
    ${${PROJECT_NAME}_event_h}
    ${${PROJECT_NAME}_base_cpp}
    ${${PROJECT_NAME}_shutil_cpp}
    ${${PROJECT_NAME}_utils_cpp}
    ${${PROJECT_NAME}_widgets_cpp}
    ${${PROJECT_NAME}_localfile_cpp}
    ${${PROJECT_NAME}_job_cpp}
    ${${PROJECT_NAME}_event_cpp}
    ${${PROJECT_NAME}_qrcs}
    )

target_link_libraries(
    ${PROJECT_NAME}
    ${QtUseModules}
    ${PkgUseModules}
    )
