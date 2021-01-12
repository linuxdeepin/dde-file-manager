#-------------------------------------------------
#
# Project created by QtCreator 2020-08-07T11:52:15
#
#-------------------------------------------------

#QT       -= core gui

#TARGET = charsetdetect
#TEMPLATE = lib

#DEFINES += CHARSETDETECT_LIBRARY

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
#DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    $$PWD/charsetdetect.cpp \
    $$PWD/mozilla/CharDistribution.cpp \
    $$PWD/mozilla/JpCntx.cpp \
    $$PWD/mozilla/LangBulgarianModel.cpp \
    $$PWD/mozilla/LangCyrillicModel.cpp \
    $$PWD/mozilla/LangGreekModel.cpp \
    $$PWD/mozilla/LangHebrewModel.cpp \
    $$PWD/mozilla/LangHungarianModel.cpp \
    $$PWD/mozilla/LangThaiModel.cpp \
    $$PWD/mozilla/nsBig5Prober.cpp \
    $$PWD/mozilla/nsCharSetProber.cpp \
    $$PWD/mozilla/nsEscCharsetProber.cpp \
    $$PWD/mozilla/nsEscSM.cpp \
    $$PWD/mozilla/nsEUCJPProber.cpp \
    $$PWD/mozilla/nsEUCKRProber.cpp \
    $$PWD/mozilla/nsEUCTWProber.cpp \
    $$PWD/mozilla/nsGB2312Prober.cpp \
    $$PWD/mozilla/nsHebrewProber.cpp \
    $$PWD/mozilla/nsLatin1Prober.cpp \
    $$PWD/mozilla/nsMBCSGroupProber.cpp \
    $$PWD/mozilla/nsMBCSSM.cpp \
    $$PWD/mozilla/nsSBCharSetProber.cpp \
    $$PWD/mozilla/nsSBCSGroupProber.cpp \
    $$PWD/mozilla/nsSJISProber.cpp \
    $$PWD/mozilla/nsUniversalDetector.cpp \
    $$PWD/mozilla/nsUTF8Prober.cpp

HEADERS += \
    $$PWD/nscore.h \
    $$PWD/charsetdetectPriv.h \
    $$PWD/charsetdetect.h \
    $$PWD/mozilla/CharDistribution.h \
    $$PWD/mozilla/JpCntx.h \
    $$PWD/mozilla/nsBig5Prober.h \
    $$PWD/mozilla/nsCharSetProber.h \
    $$PWD/mozilla/nsCodingStateMachine.h \
    $$PWD/mozilla/nsEscCharsetProber.h \
    $$PWD/mozilla/nsEUCJPProber.h \
    $$PWD/mozilla/nsEUCKRProber.h \
    $$PWD/mozilla/nsEUCTWProber.h \
    $$PWD/mozilla/nsGB2312Prober.h \
    $$PWD/mozilla/nsHebrewProber.h \
    $$PWD/mozilla/nsLatin1Prober.h \
    $$PWD/mozilla/nsMBCSGroupProber.h \
    $$PWD/mozilla/nsPkgInt.h \
    $$PWD/mozilla/nsSBCharSetProber.h \
    $$PWD/mozilla/nsSBCSGroupProber.h \
    $$PWD/mozilla/nsSJISProber.h \
    $$PWD/mozilla/nsUniversalDetector.h \
    $$PWD/mozilla/nsUTF8Prober.h \
    $$PWD/nspr-emu/obsolete/protypes.h \
    $$PWD/nspr-emu/prcpucfg.h \
    $$PWD/nspr-emu/prcpucfg_freebsd.h \
    $$PWD/nspr-emu/prcpucfg_linux.h \
    $$PWD/nspr-emu/prcpucfg_mac.h \
    $$PWD/nspr-emu/prcpucfg_openbsd.h \
    $$PWD/nspr-emu/prcpucfg_win.h \
    $$PWD/nspr-emu/prmem.h \
    $$PWD/nspr-emu/prtypes.h

INCLUDEPATH += $$PWD \
    $$PWD/nspr-emu \
    $$PWD/nspr-emu/obsolete \
    $$PWD/mozilla

#unix {
#    target.path = /usr/lib
#    INSTALLS += target
#}
