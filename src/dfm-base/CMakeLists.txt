cmake_minimum_required(VERSION 3.1.0)

project(dfm-base)

set(${PROJECT_NAME}_base_cpp
    base/private/subscriber.cpp
    base/private/dfilesystemwatcher.cpp
    base/private/dfminfocachesmanager.cpp
    base/private/dfmwatchercachesmanager.cpp
    base/dabstractfilecontroller.cpp
    base/dabstractfileinfo.cpp
    base/define.cpp
    base/dfmapplication.cpp
    base/dfmglobal.cpp
    base/dfmsettings.cpp
    base/dfmurlroute.cpp
    base/dfmstandardpaths.cpp
    base/sort.cpp
    base/dfmschemefactory.cpp
    base/dabstractfilewatcher.cpp
    base/dabstractfiledevice.cpp
    base/dabstractjobhandler.cpp)

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

set(${PROJECT_NAME}_utils_cpp
    utils/utils.cpp
    utils/desktopinfo.cpp
    )

set(${PROJECT_NAME}_widgets_cpp
    widgets/dfmsidebar/dfmsidebar.cpp
    widgets/dfmsidebar/dfmsidebaritem.cpp
    widgets/dfmsidebar/dfmsidebaritemdelegate.cpp
    widgets/dfmsidebar/dfmsidebarmodel.cpp
    widgets/dfmsidebar/dfmsidebarview.cpp
    widgets/dfmsidebar/dfmsidebarmanager.cpp
    widgets/dfmsplitter/dfmsplitter.cpp
    widgets/dfmfileview/dfmfileview.cpp
    widgets/dfmfileviewicondelegate.cpp
    widgets/dfmfileview/dfmfileviewmodel.cpp
    widgets/dfmfileview/dfmheaderview.cpp
    widgets/dfmfileview/dfmfileviewitem.cpp
    widgets/dfmfileview/private/dfmtraversaldirthread.cpp
    widgets/dfmfileview/dfmlistitemdelegate.cpp
    widgets/dfmfileview/dfmstyleditemdelegate.cpp
    widgets/dfmfileview/dfmiconitemdelegate.cpp)

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
    localfile/dfmgiofiledevice.cpp
    localfile/dlocalfilehandler.cpp
    )

set(${PROJECT_NAME}_job_cpp
    job/jobcontroller.cpp
    )

set(${PROJECT_NAME}_event_cpp
    event/dfmglobaleventdispatcher.cpp
    event/protocol/dfmevent.cpp
    event/protocol/dfmfileevent.cpp
    event/protocol/dfmlauchevent.cpp
    event/protocol/dfmwindowevent.cpp
    event/handler/dfmabstracteventhandler.cpp
    event/handler/dfmfileeventhandler.cpp
    event/handler/dfmlaucheventhandler.cpp
    event/handler/dfmwindoweventhandler.cpp
    event/dfmglobaleventmessage.cpp
    )

set(${PROJECT_NAME}_base_h
    #    base/appcontroller.h
    base/private/dfilesystemwatcher_p.h
    base/private/dabstractfilewatcher_p.h
    base/private/dabstractfiledevice_p.h
    base/private/dabstractfileinfo_p.h
    base/private/dfilehandler_p.h
    base/private/dfmapplication_p.h
    base/private/subscriber.h
    base/private/dfminfocachesmanager.h
    base/private/dfminfocachesmanager_p.h
    base/private/dfmwatchercachesmanager.h
    base/private/dfmwatchercachesmanager_p.h
    base/private/dfmthreadcontainer.hpp
    base/dabstractfilecontroller.h
    base/dabstractfileinfo.h
    base/define.h
    base/dfmapplication.h
    base/dfmglobal.h
    base/dfmsettings.h
    base/dfmstandardpaths.h
    base/singleton.h
    base/dfmurlroute.h
    base/dfmschemefactory.h
    base/dabstractdiriterator.h
    base/dfmschemefactory.h
    base/dabstractfilewatcher.h
    base/dabstractfiledevice.h
    base/dabstractjobhandler.cpp
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

set(${PROJECT_NAME}_utils_h
    utils/utils.h
    utils/desktopinfo.h
    )

set(${PROJECT_NAME}_widgets_h
    widgets/dfmsidebar/dfmsidebar.h
    widgets/dfmsidebar/dfmsidebaritem.h
    widgets/dfmsidebar/dfmsidebaritemdelegate.h
    widgets/dfmsidebar/dfmsidebarmodel.h
    widgets/dfmsidebar/dfmsidebarview.h
    widgets/dfmsidebar/dfmsidebarmanager.h
    widgets/dfmsplitter/dfmsplitter.h
    widgets/dfmfileview/dfmfileview.h
    widgets/dfmfileviewicondelegate.h
    widgets/dfmfileview/dfmfileviewmodel.h
    widgets/dfmfileview/dfmheaderview.h
    widgets/dfmfileview/dfmfileviewitem.h
    widgets/dfmfileview/private/dfmfileview_p.h
    widgets/dfmfileview/private/dfmfileviewmodel_p.h
    widgets/dfmfileview/private/dfmtraversaldirthread.h
    widgets/dfmfileview/dfmlistitemdelegate.h
    widgets/dfmfileview/dfmstyleditemdelegate.h
    widgets/dfmfileview/dfmstyleditemdelegate_p.h
    widgets/dfmfileview/dfmiconitemdelegate.h
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
    localfile/dfmgiofiledevice.h
    localfile/dfmlocalfiledevice.h
    localfile/private/dfmlocalfiledevice_p.h
    )

set(${PROJECT_NAME}_job_h
    job/jobcontroller.h
    )

set(${PROJECT_NAME}_event_h
    event/dfmglobaleventdispatcher.h
    event/protocol/dfmevent.h
    event/protocol/dfmfileevent.h
    event/protocol/dfmlauchevent.h
    event/protocol/dfmwindowevent.h
    event/handler/dfmabstracteventhandler.h
    event/handler/dfmfileeventhandler.h
    event/handler/dfmlaucheventhandler.h
    event/handler/dfmwindoweventhandler.h
    event/dfmglobaleventmessage.h
    )

set(${PROJECT_NAME}_qrcs
    resources/resources.qrc
    )

add_library(${PROJECT_NAME}
    SHARED
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
