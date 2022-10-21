INCLUDEPATH += $$PWD

#isEmpty(GOOGLETEST_DIR):GOOGLETEST_DIR=$$(GOOGLETEST_DIR)

#isEmpty(GOOGLETEST_DIR) {
#    warning("Using googletest src dir specified at Qt Creator wizard")
#    message("set GOOGLETEST_DIR as environment variable or qmake variable to get rid of this message")
#    GOOGLETEST_DIR = $$PWD
#}

#!isEmpty(GOOGLETEST_DIR): {
#    GTEST_SRCDIR = $$GOOGLETEST_DIR/googletest
#    GMOCK_SRCDIR = $$GOOGLETEST_DIR/googlemock
#}

#requires(exists($$GTEST_SRCDIR):exists($$GMOCK_SRCDIR))

#!exists($$GOOGLETEST_DIR):message("No googletest src dir found - set GOOGLETEST_DIR to enable.")


#INCLUDEPATH *= \
#    $$GTEST_SRCDIR \
#    $$GTEST_SRCDIR/include \
#    $$GMOCK_SRCDIR \
#    $$GMOCK_SRCDIR/include

INCLUDEPATH += $$PWD/../stub-ext
SOURCES += $$PWD/elfio.hpp \
           $$PWD/../stub-ext/stub-shadow.cpp

HEADERS += $$PWD/addr_any.h \
            $$PWD/addr_pri.h \
            $$PWD/stub.h \
            $$PWD/../stub-ext/stubext.h \
            $$PWD/../stub-ext/stub-shadow.h
