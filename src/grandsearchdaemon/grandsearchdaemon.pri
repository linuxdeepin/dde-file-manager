SOURCES += \
    $$PWD/daemonlibrary.cpp \
    $$PWD/daemonplugin.cpp

HEADERS  += \ 
    $$PWD/daemonlibrary.h \
    $$PWD/daemonplugin.h

INCLUDEPATH += $$PWD

GRANDSEARCHDAEMON_LIB_DIR = $$[QT_INSTALL_LIBS]/dde-grand-search-daemon

DEFINES += GRANDSEARCHDAEMON_LIB_DIR=\\\"$$GRANDSEARCHDAEMON_LIB_DIR\\\"

message(GRANDSEARCHDAEMON_LIB_DIR:$$GRANDSEARCHDAEMON_LIB_DIR)
