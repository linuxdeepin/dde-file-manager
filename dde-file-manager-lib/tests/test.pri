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
