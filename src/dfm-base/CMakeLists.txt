cmake_minimum_required(VERSION 3.1.0)

project(dfm-base)

set(${PROJECT_NAME}_global_h
    dfm_base_global.h)

set(${PROJECT_NAME}_utils_h
    utils/utils.h
    utils/desktopinfo.h
    utils/threadcontainer.hpp
    )

set(${PROJECT_NAME}_utils_cpp
    utils/utils.cpp
    utils/desktopinfo.cpp
    )


set(${PROJECT_NAME}_shutil_h
    shutil/checknetwork.h
    shutil/dfmfilelistfile.h
    shutil/dfmregularexpression.h
    shutil/dsqlitehandle.h
    shutil/fileutils.h
    shutil/properties.h
    shutil/shortcut.h
    shutil/dmimedatabase.h
    )

set(${PROJECT_NAME}_shutil_cpp
    shutil/checknetwork.cpp
    shutil/dfmfilelistfile.cpp
    shutil/dfmregularexpression.cpp
    shutil/dsqlitehandle.cpp
    shutil/fileutils.cpp
    shutil/properties.cpp
    shutil/shortcut.cpp
    shutil/dmimedatabase.cpp
    )

set(${PROJECT_NAME}_base_h
    base/private/dfilesystemwatcher_p.h
    base/private/dabstractfilewatcher_p.h
    base/private/dabstractfiledevice_p.h
    base/private/dabstractfileinfo_p.h
    base/private/dfmapplication_p.h
    base/private/subscriber.h
    base/private/dfminfocache.h
    base/private/dfminfocache_p.h
    base/private/dfmwatchercache.h
    base/private/dfmwatchercache_p.h
    base/dabstractfilecontroller.h
    base/dabstractfileinfo.h
    base/dfmapplication.h
    base/dfmsettings.h
    base/dfmstandardpaths.h
    base/singleton.hpp
    base/dfmurlroute.h
    base/dfmschemefactory.h
    base/dabstractdiriterator.h
    base/dfmschemefactory.h
    base/dabstractfilewatcher.h
    base/dabstractfiledevice.h
    base/sort.h
    )

set(${PROJECT_NAME}_base_cpp
    base/private/subscriber.cpp
    base/private/dfilesystemwatcher.cpp
    base/private/dfminfocache.cpp
    base/private/dfmwatchercache.cpp
    base/dabstractfilecontroller.cpp
    base/dabstractfileinfo.cpp
    base/dfmapplication.cpp
    base/dfmsettings.cpp
    base/dfmurlroute.cpp
    base/dfmstandardpaths.cpp
    base/sort.cpp
    base/dfmschemefactory.cpp
    base/dabstractfilewatcher.cpp
    base/dabstractfiledevice.cpp
    )


set(${PROJECT_NAME}_widgets_cpp
    widgets/dfmsidebar/sidebar.cpp
    widgets/dfmsidebar/sidebaritem.cpp
    widgets/dfmsidebar/sidebaritemdelegate.cpp
    widgets/dfmsidebar/sidebarmodel.cpp
    widgets/dfmsidebar/sidebarview.cpp
    widgets/dfmsidebar/sidebarmanager.cpp
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
    #    localfile/pathmanager.h
    #    localfile/filesignalmanager.h
    localfile/filecontroller.h
    #    localfile/dfileservices.h
    localfile/dfilehandler.h
    localfile/private/dfilehandler_p.h
    localfile/dfmfilecontrollerfactory.h
    localfile/dfmlocalfileinfo.h
    localfile/dfmlocalmenu.h
    localfile/private/dfmlocalmenu_p.h
    localfile/private/dfmlocalfileinfo_p.h
    localfile/private/dfmlocaldiriterator_p.h
    localfile/dfmlocaldiriterator.h
    localfile/dfmlocalfileinfo.h
    localfile/private/dfmlocalfileinfo_p.h
    localfile/dfmlocalfiledevice.h
    localfile/private/dfmlocalfiledevice_p.h
    )

set(${PROJECT_NAME}_localfile_cpp
    #    localfile/pathmanager.cpp
    #    localfile/filesignalmanager.cpp
    localfile/filecontroller.cpp
    localfile/dfmfilecontrollerfactory.cpp
    #    localfile/dfileservices.cpp
    localfile/dfilehandler.cpp
    localfile/dfmlocalfileinfo.cpp
    localfile/dfmlocalmenu.cpp
    localfile/dfmlocaldiriterator.cpp
    localfile/dfmlocalfiledevice.cpp
    localfile/dlocalfilehandler.cpp
    )

set(${PROJECT_NAME}_widgets_h
    widgets/dfmsidebar/sidebar.h
    widgets/dfmsidebar/sidebaritem.h
    widgets/dfmsidebar/sidebaritemdelegate.h
    widgets/dfmsidebar/sidebarmodel.h
    widgets/dfmsidebar/sidebarview.h
    widgets/dfmsidebar/sidebarmanager.h
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
    widgets/dfmfileview/styleditemdelegate_p.h
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
