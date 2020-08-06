#-------------------------------------------------
#
# Project created by QtCreator 2015-06-24T09:14:17
#
#-------------------------------------------------
#system($$PWD/../vendor/prebuild)
#include($$PWD/../vendor/vendor.pri)

INCLUDEPATH += $$PWD

HEADERS += \
    $$PWD/interfaces/ut_durl_test.h \
    $$PWD/interfaces/tst_all_interfaces.h \

SOURCES += \
    $$PWD/main.cpp \
    # vault
    $$PWD/vault/ut_interfaceactivevault.cpp \
    $$PWD/vault/ut_operatorcenter.cpp \
    $$PWD/vault/ut_vaulthelper.cpp \
    $$PWD/vault/ut_vaultlockmanager.cpp \
    $$PWD/interfaces/ut_dfmglobal.cpp \
    $$PWD/dialogs/ut_dialogmanager.cpp \
    $$PWD/models/ut_avfsfileinfo.cpp \
    $$PWD/models/ut_computermodel.cpp \
    $$PWD/models/ut_dfmrootfileinfo.cpp \
    $$PWD/models/ut_bookmark.cpp \
    $$PWD/models/ut_desktopfileinfo.cpp \
    $$PWD/models/ut_deviceinfoparser.cpp \
    $$PWD/models/ut_dfileselectionmodel.cpp \
    $$PWD/models/ut_dfmsidebarmodel.cpp \
    $$PWD/models/ut_masteredmediafileinfo.cpp \
    $$PWD/models/ut_mergeddesktopfileinfo.cpp \
    $$PWD/models/ut_mountfileinfo.cpp \
    $$PWD/models/ut_networkfileinfo.cpp \
    $$PWD/models/ut_recentfileinfo.cpp \
    $$PWD/models/ut_searchfileinfo.cpp \
    $$PWD/models/ut_searchhistory.cpp \
    $$PWD/models/ut_sharefileinfo.cpp
