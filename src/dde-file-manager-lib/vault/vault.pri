INCLUDEPATH += $$PWD
DEFINES += HAVE_CONFIG_H

SOURCES += \
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
    $$PWD/openssl/rsam.cpp \
    $$PWD/interfaceactivevault.cpp \
    $$PWD/operatorcenter.cpp \
    $$PWD/vaultlockmanager.cpp \
    $$PWD/vaulthelper.cpp \
    $$PWD/vaultconfig.cpp

HEADERS += \
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
    $$PWD/openssl/rsam.h \
    $$PWD/interfaceactivevault.h \
    $$PWD/operatorcenter.h \
    $$PWD/vaultglobaldefine.h \
    $$PWD/vaultlockmanager.h \
    $$PWD/vaulthelper.h \
    $$PWD/vaultconfig.h

PKGCONFIG += libcrypto

#LIBS += /usr/lib/x86_64-linux-gnu/libssl.so \
#        /usr/lib/x86_64-linux-gnu/libcrypto.so

RESOURCES +=

