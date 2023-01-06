3RD_UTILS = $$PWD/../../3rdparty/utils

HEADERS += \
    $$PWD/utils.h \
    $$PWD/xutil.h \
    $$PWD/debugobejct.h \
    $$PWD/singleton.h \
    $$PWD/cryptoutils.h \
    $$PWD/desktopinfo.h \
    $$PWD/grouppolicy.h \
    $$PWD/rlog/rlog.h \
    $$PWD/rlog/committhread.h \
    $$PWD/rlog/datas/reportdatainterface.h \
    $$PWD/rlog/datas/blockmountreportdata.h \
    $$PWD/rlog/datas/smbreportdata.h \
    $$PWD/rlog/datas/vaultreportdata.h \
    $$PWD/rlog/datas/searchreportdata.h \
    $$PWD/rlog/datas/sidebarreportdata.h \
    $$PWD/rlog/datas/filemenureportdata.h \
    $$PWD/rlog/datas/appstartupreportdata.h \
    $$3RD_UTILS/qobjecthelper.h

SOURCES += \
    $$PWD/utils.cpp \
    $$PWD/xutil.cpp \
    $$PWD/debugobejct.cpp \
    $$PWD/cryptoutils.cpp \
    $$PWD/desktopinfo.cpp \
    $$PWD/grouppolicy.cpp \
    $$PWD/rlog/rlog.cpp \
    $$PWD/rlog/committhread.cpp \
    $$PWD/rlog/datas/blockmountreportdata.cpp \
    $$PWD/rlog/datas/smbreportdata.cpp \
    $$PWD/rlog/datas/vaultreportdata.cpp \
    $$PWD/rlog/datas/searchreportdata.cpp \
    $$PWD/rlog/datas/sidebarreportdata.cpp \
    $$PWD/rlog/datas/filemenureportdata.cpp \
    $$PWD/rlog/datas/appstartupreportdata.cpp \
    $$3RD_UTILS/qobjecthelper.cpp
