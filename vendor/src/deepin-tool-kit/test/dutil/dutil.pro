QT += testlib
QT -= gui

CONFIG += testcase

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../../dutil/release/ -ldtkutil
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../../dutil/debug/ -ldtkutil
else:unix: LIBS += -L$$OUT_PWD/../../dutil/ -ldtkutil

INCLUDEPATH += $$PWD/../../dutil
DEPENDPATH += $$PWD/../../dutil

SOURCES += \
    dutiltester.cpp \
    main.cpp

HEADERS += \
    dutiltester.h

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../../dbase/release/ -ldtkbase
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../../dbase/debug/ -ldtkbase
else:unix: LIBS += -L$$OUT_PWD/../../dbase/ -ldtkbase

INCLUDEPATH += $$PWD/../../dbase
DEPENDPATH += $$PWD/../../dbase
