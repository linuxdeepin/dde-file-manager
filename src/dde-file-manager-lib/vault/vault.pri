DEFINES += HAVE_CONFIG_H

3rd_srcdir = $$PWD/../../../3rdparty

SOURCES += \
    $$3rd_srcdir/qrencode/bitstream.c \
    $$3rd_srcdir/qrencode/mask.c \
    $$3rd_srcdir/qrencode/mmask.c \
    $$3rd_srcdir/qrencode/mqrspec.c \
    $$3rd_srcdir/qrencode/qrencode.c \
    $$3rd_srcdir/qrencode/qrinput.c \
    $$3rd_srcdir/qrencode/rscode.c \
    $$3rd_srcdir/qrencode/split.c \
    $$3rd_srcdir/qrencode/qrspec.c \
    $$PWD/openssl-operator/pbkdf2.cpp \
    $$PWD/openssl-operator/rsam.cpp \
    $$PWD/interfaceactivevault.cpp \
    $$PWD/operatorcenter.cpp \
    $$PWD/vaultlockmanager.cpp \
    $$PWD/vaulthelper.cpp \
    $$PWD/vaultconfig.cpp

HEADERS += \
    $$3rd_srcdir/qrencode/bitstream.h \
    $$3rd_srcdir/qrencode/config.h \
    $$3rd_srcdir/qrencode/mask.h \
    $$3rd_srcdir/qrencode/mmask.h \
    $$3rd_srcdir/qrencode/mqrspec.h \
    $$3rd_srcdir/qrencode/qrencode_inner.h \
    $$3rd_srcdir/qrencode/qrencode.h \
    $$3rd_srcdir/qrencode/qrinput.h \
    $$3rd_srcdir/qrencode/rscode.h \
    $$3rd_srcdir/qrencode/split.h \
    $$3rd_srcdir/qrencode/qrspec.h \
    $$PWD/openssl-operator/pbkdf2.h \
    $$PWD/openssl-operator/rsam.h \
    $$PWD/interfaceactivevault.h \
    $$PWD/operatorcenter.h \
    $$PWD/vaultglobaldefine.h \
    $$PWD/vaultlockmanager.h \
    $$PWD/vaulthelper.h \
    $$PWD/vaultconfig.h

PKGCONFIG += libcrypto

RESOURCES +=

INCLUDEPATH += $$PWD \
               $$3rd_srcdir

