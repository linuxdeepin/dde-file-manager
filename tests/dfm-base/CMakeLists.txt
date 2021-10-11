cmake_minimum_required(VERSION 3.1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall -Wl,--as-need -fPIE")

# 定义可执行程序名称
project(test-dfm-base)

# 源工程文件
set(SRC_DIR ${PROJECT_SOURCE_PATH}/dfm-base)
include_directories(${SRC_DIR}) #include source dir

set(GLOBAL_H
    ${SRC_DIR}/dfm_base_global.h)

set(UTILS_H
    ${SRC_DIR}/utils/threadcontainer.hpp
    ${SRC_DIR}/utils/fileutils.h
    ${SRC_DIR}/utils/iconutils.h
    )

set(UTILS_CPP
    ${SRC_DIR}/utils/fileutils.cpp
    )


set(MIMETYPE_H
    ${SRC_DIR}/mimetype/mimedatabase.h
    )

set(MIMETYPE_H
    ${SRC_DIR}/mimetype/mimedatabase.cpp
    )

set(BASE_H
    ${SRC_DIR}/base/private/abstractfilewatcher_p.h
    ${SRC_DIR}/base/private/abstractfiledevice_p.h
    ${SRC_DIR}/base/private/abstractfileinfo_p.h
    ${SRC_DIR}/base/private/subscriber.h
    ${SRC_DIR}/base/private/infocache.h
    ${SRC_DIR}/base/private/infocache_p.h
    ${SRC_DIR}/base/private/watchercache.h
    ${SRC_DIR}/base/private/watchercache_p.h
    ${SRC_DIR}/base/abstractfileinfo.h
    ${SRC_DIR}/base/standardpaths.h
    ${SRC_DIR}/base/singleton.h
    ${SRC_DIR}/base/urlroute.h
    ${SRC_DIR}/base/schemefactory.h
    ${SRC_DIR}/base/abstractdiriterator.h
    ${SRC_DIR}/base/schemefactory.h
    ${SRC_DIR}/base/abstractfilewatcher.h
    ${SRC_DIR}/base/abstractfiledevice.h
    )

set(BASE_CPP
    ${SRC_DIR}/base/private/subscriber.cpp
    ${SRC_DIR}/base/private/infocache.cpp
    ${SRC_DIR}/base/private/watchercache.cpp
    ${SRC_DIR}/base/abstractfileinfo.cpp
    ${SRC_DIR}/base/urlroute.cpp
    ${SRC_DIR}/base/standardpaths.cpp
    ${SRC_DIR}/base/schemefactory.cpp
    ${SRC_DIR}/base/abstractfilewatcher.cpp
    ${SRC_DIR}/base/abstractfiledevice.cpp
    )

set(LOCALFILE_H
    ${SRC_DIR}/localfile/private/localmenu_p.h
    ${SRC_DIR}/localfile/private/localdiriterator_p.h
    ${SRC_DIR}/localfile/private/localfileinfo_p.h
    ${SRC_DIR}/localfile/private/localfilewatcher_p.h
    ${SRC_DIR}/localfile/localdiriterator.h
    ${SRC_DIR}/localfile/localfileinfo.h
    ${SRC_DIR}/localfile/localmenu.h
    ${SRC_DIR}/localfile/localfilewatcher.h
    )

set(LOCALFILE_CPP
    ${SRC_DIR}/localfile/localfileinfo.cpp
    ${SRC_DIR}/localfile/localmenu.cpp
    ${SRC_DIR}/localfile/localdiriterator.cpp
    ${SRC_DIR}/localfile/localfilewatcher.cpp
    )

set(WIDGETS_H
    ${SRC_DIR}/widgets/dfmfileview/private/fileview_p.h
    ${SRC_DIR}/widgets/dfmfileview/private/fileviewmodel_p.h
    ${SRC_DIR}/widgets/dfmfileview/private/traversaldirthread.h
    ${SRC_DIR}/widgets/dfmfileview/private/fileviewitem_p.h
    ${SRC_DIR}/widgets/dfmfileview/private/delegatecommon.h
    ${SRC_DIR}/widgets/dfmfileview/private/iconitemdelegate_p.h
    ${SRC_DIR}/widgets/dfmfileview/private/listitemdelegate_p.h
    ${SRC_DIR}/widgets/dfmsidebar/private/sidebar_p.h
    ${SRC_DIR}/widgets/dfmsidebar/private/sidebarview_p.h
    ${SRC_DIR}/widgets/dfmsidebar/private/sidebarmodel_p.h
    ${SRC_DIR}/widgets/dfmsidebar/sidebar.h
    ${SRC_DIR}/widgets/dfmsidebar/sidebaritem.h
    ${SRC_DIR}/widgets/dfmsidebar/sidebaritemdelegate.h
    ${SRC_DIR}/widgets/dfmsidebar/sidebarmodel.h
    ${SRC_DIR}/widgets/dfmsidebar/sidebarview.h
    ${SRC_DIR}/widgets/dfmsplitter/splitter.h
    ${SRC_DIR}/widgets/dfmfileview/fileview.h
    ${SRC_DIR}/widgets/dfmfileview/fileviewmodel.h
    ${SRC_DIR}/widgets/dfmfileview/headerview.h
    ${SRC_DIR}/widgets/dfmfileview/fileviewitem.h
    ${SRC_DIR}/widgets/dfmfileview/listitemdelegate.h
    ${SRC_DIR}/widgets/dfmfileview/iconitemdelegate.h
    )

set(WIDGETS_CPP
    ${SRC_DIR}/widgets/dfmfileview/private/traversaldirthread.cpp
    ${SRC_DIR}/widgets/dfmfileview/private/fileview_p.cpp
    ${SRC_DIR}/widgets/dfmfileview/private/delegatecommon.cpp
    ${SRC_DIR}/widgets/dfmfileview/private/iconitemdelegate_p.cpp
    ${SRC_DIR}/widgets/dfmfileview/private/listitemdelegate_p.cpp
    ${SRC_DIR}/widgets/dfmsidebar/private/sidebar_p.cpp
    ${SRC_DIR}/widgets/dfmsidebar/sidebar.cpp
    ${SRC_DIR}/widgets/dfmsidebar/sidebaritem.cpp
    ${SRC_DIR}/widgets/dfmsidebar/sidebaritemdelegate.cpp
    ${SRC_DIR}/widgets/dfmsidebar/sidebarmodel.cpp
    ${SRC_DIR}/widgets/dfmsidebar/sidebarview.cpp
    ${SRC_DIR}/widgets/dfmsplitter/splitter.cpp
    ${SRC_DIR}/widgets/dfmfileview/fileview.cpp
    ${SRC_DIR}/widgets/dfmfileview/fileviewmodel.cpp
    ${SRC_DIR}/widgets/dfmfileview/headerview.cpp
    ${SRC_DIR}/widgets/dfmfileview/fileviewitem.cpp
    ${SRC_DIR}/widgets/dfmfileview/listitemdelegate.cpp
    ${SRC_DIR}/widgets/dfmfileview/iconitemdelegate.cpp
    )

set(QRCS
    ${SRC_DIR}/resources/resources.qrc
    )

# UT文件
file(GLOB_RECURSE UT_CXX_FILE
     FILES_MATCHING PATTERN "*.cpp") #public dir

add_executable(${PROJECT_NAME}
    main.cpp
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
    ${QRCS}
    ${UT_CXX_FILE}
    ${CPP_STUB_SRC}
    )

message(PkgUseModules ${PkgUseModules})

target_link_libraries(
    ${PROJECT_NAME}
    ${QtUseModules}
    ${KdeFindPackage}
    ${PkgUseModules}
    )

