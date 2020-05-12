INCLUDEPATH += $$PWD
DEFINES += HAVE_CONFIG_H

SOURCES += \
    $$PWD/activevault/widgetstartvault.cpp \
    $$PWD/activevault/widgetsetunlockmethod.cpp \
    $$PWD/activevault/widgetsavekey.cpp \
    $$PWD/activevault/widgetactivcevaultfinished.cpp \
    $$PWD/qrencode/bitstream.c \
    $$PWD/qrencode/mask.c \
    $$PWD/qrencode/mmask.c \
    $$PWD/qrencode/mqrspec.c \
    $$PWD/qrencode/qrencode.c \
    $$PWD/qrencode/qrinput.c \
    $$PWD/qrencode/rscode.c \
    $$PWD/qrencode/split.c \
    $$PWD/qrencode/qrspec.c \
    $$PWD/openssl/pbkdf2.cpp \
    $$PWD/activevault/widgetmanager.cpp \
    $$PWD/openssl/rsam.cpp \
    $$PWD/interfaceactivevault.cpp \
    $$PWD/operatorcenter.cpp

HEADERS += \
    $$PWD/activevault/widgetstartvault.h \
    $$PWD/activevault/widgetsetunlockmethod.h \
    $$PWD/activevault/widgettooltips.h \
    $$PWD/activevault/widgetsavekey.h \
    $$PWD/activevault/widgetactivcevaultfinished.h \
    $$PWD/activevault/widgetshowqrcode.h \
    $$PWD/activevault/dialogframeless.h \
    $$PWD/activevault/dialogautocreatepassword.h \
    $$PWD/qrencode/bitstream.h \
    $$PWD/qrencode/config.h \
    $$PWD/qrencode/mask.h \
    $$PWD/qrencode/mmask.h \
    $$PWD/qrencode/mqrspec.h \
    $$PWD/qrencode/qrencode_inner.h \
    $$PWD/qrencode/qrencode.h \
    $$PWD/qrencode/qrinput.h \
    $$PWD/qrencode/rscode.h \
    $$PWD/qrencode/split.h \
    $$PWD/qrencode/qrspec.h \
    $$PWD/openssl/pbkdf2.h \
    $$PWD/activevault/widgetmanager.h \
    $$PWD/openssl/rsam.h \
    $$PWD/interfaceactivevault.h \
    $$PWD/operatorcenter.h \
    $$PWD/vaultglobaldefine.h

#TRANSLATIONS += \
#        $$PWD/activevault/language/chinese.ts

LIBS += /usr/lib/x86_64-linux-gnu/libssl.so \
        /usr/lib/x86_64-linux-gnu/libcrypto.so

RESOURCES +=

