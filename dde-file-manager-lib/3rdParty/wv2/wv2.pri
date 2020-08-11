#-------------------------------------------------
#
# Project created by QtCreator 2020-08-07T10:26:55
#
#-------------------------------------------------

CONFIG += c++11 link_pkgconfig
PKGCONFIG += libgsf-1 glib-2.0

DEFINES += HAVE_CONFIG_H

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
    $$PWD/associatedstrings.cpp \
    $$PWD/convert.cpp \
    $$PWD/fields.cpp \
    $$PWD/fonts.cpp \
    $$PWD/footnotes97.cpp \
    $$PWD/functor.cpp \
    $$PWD/functordata.cpp \
    $$PWD/global.cpp \
    $$PWD/handlers.cpp \
    $$PWD/headers.cpp \
    $$PWD/headers95.cpp \
    $$PWD/headers97.cpp \
    $$PWD/lists.cpp \
    $$PWD/olestream.cpp \
    $$PWD/paragraphproperties.cpp \
    $$PWD/parser.cpp \
    $$PWD/parser9x.cpp \
    $$PWD/parser95.cpp \
    $$PWD/parser97.cpp \
    $$PWD/parserfactory.cpp \
    $$PWD/properties97.cpp \
    $$PWD/styles.cpp \
    $$PWD/textconverter.cpp \
    $$PWD/ustring.cpp \
    $$PWD/word_helper.cpp \
    $$PWD/word95_generated.cpp \
    $$PWD/word95_helper.cpp \
    $$PWD/word97_generated.cpp \
    $$PWD/word97_helper.cpp \
    $$PWD/wvlog.cpp \
    $$PWD/crc32.c \
    $$PWD/utilitiesrename.cpp

HEADERS += \
    $$PWD/associatedstrings.h \
    $$PWD/convert.h \
    $$PWD/crc32.h \
    $$PWD/dllmagic.h \
    $$PWD/fields.h \
    $$PWD/fonts.h \
    $$PWD/footnotes97.h \
    $$PWD/functor.h \
    $$PWD/functordata.h \
    $$PWD/global.h \
    $$PWD/handlers.h \
    $$PWD/headers.h \
    $$PWD/headers95.h \
    $$PWD/headers97.h \
    $$PWD/lists.h \
    $$PWD/olestorage.h \
    $$PWD/olestream.h \
    $$PWD/paragraphproperties.h \
    $$PWD/parser.h \
    $$PWD/parser9x.h \
    $$PWD/parser95.h \
    $$PWD/parser97.h \
    $$PWD/parserfactory.h \
    $$PWD/properties97.h \
    $$PWD/sharedptr.h \
    $$PWD/styles.cpp.orig \
    $$PWD/styles.h \
    $$PWD/textconverter.h \
    $$PWD/ustring.h \
    $$PWD/word_helper.h \
    $$PWD/word95_generated.h \
    $$PWD/word95_helper.h \
    $$PWD/word97_generated.h \
    $$PWD/word97_helper.h \
    $$PWD/wvlog.h \
    $$PWD/config.h \
    $$PWD/utilitiesrename.h

#unix {
#    target.path = /usr/lib
#    INSTALLS += target
#}
