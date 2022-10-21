include(../../common/common.pri)

3RDPART_DIR = $$PWD/../../../3rdpart

HEADERS += \
    $$PWD/maincontroller/task/taskcommander.h \
    $$PWD/maincontroller/task/taskcommander_p.h \
    $$PWD/maincontroller/maincontroller.h \
    $$PWD/private/searchservice_p.h \
    $$3RDPART_DIR/fulltext/chineseanalyzer.h \
    $$3RDPART_DIR/fulltext/chinesetokenizer.h \
    $$PWD/searcher/fulltext/fulltextsearcher.h \
    $$PWD/searcher/fulltext/fulltextsearcher_p.h \
    $$PWD/searcher/iterator/iteratorsearcher.h \
    $$PWD/searcher/abstractsearcher.h \
    $$PWD/utils/searchhelper.h \
    $$PWD/searchservice.h \
    $$PWD/searcher/fsearch/fssearcher.h \
#    -----------fsearch source---------------
    $$3RDPART_DIR/fsearch/query.h \
    $$3RDPART_DIR/fsearch/array.h \
    $$3RDPART_DIR/fsearch/btree.h \
    $$3RDPART_DIR/fsearch/database_search.h \
    $$3RDPART_DIR/fsearch/database.h \
    $$3RDPART_DIR/fsearch/fsearch_config.h \
    $$3RDPART_DIR/fsearch/fsearch_limits.h \
    $$3RDPART_DIR/fsearch/fsearch_thread_pool.h \
    $$3RDPART_DIR/fsearch/fsearch.h \
    $$3RDPART_DIR/fsearch/string_utils.h \
    $$3RDPART_DIR/fsearch/utf8.h
#    -----------fsearch source---------------

SOURCES += \
    $$PWD/maincontroller/task/taskcommander.cpp \
    $$PWD/maincontroller/maincontroller.cpp \
    $$3RDPART_DIR/fulltext/chineseanalyzer.cpp \
    $$3RDPART_DIR/fulltext/chinesetokenizer.cpp \
    $$PWD/searcher/fulltext/fulltextsearcher.cpp \
    $$PWD/searcher/iterator/iteratorsearcher.cpp \
    $$PWD/searcher/abstractsearcher.cpp \
    $$PWD/utils/searchhelper.cpp \
    $$PWD/searchservice.cpp \
    $$PWD/searcher/fsearch/fssearcher.cpp \
#    -----------fsearch source---------------
    $$3RDPART_DIR/fsearch/array.c \
    $$3RDPART_DIR/fsearch/btree.c \
    $$3RDPART_DIR/fsearch/database_search.c \
    $$3RDPART_DIR/fsearch/database.c \
    $$3RDPART_DIR/fsearch/fsearch_config.c \
    $$3RDPART_DIR/fsearch/fsearch_thread_pool.c \
    $$3RDPART_DIR/fsearch/fsearch.c \
    $$3RDPART_DIR/fsearch/query.c \
    $$3RDPART_DIR/fsearch/string_utils.c
#    -----------fsearch source---------------

CONFIG(ENABLE_ANYTHING) {
message("Can support deepin-anything to search.")
HEADERS += \
    $$PWD/searcher/anything/anythingsearcher.h

SOURCES += \
    $$PWD/searcher/anything/anythingsearcher.cpp
}

INCLUDEPATH += \
    $$PWD \
    $$PWD/searcher

CONFIG += link_pkgconfig
PKGCONFIG += liblucene++ liblucene++-contrib docparser
LIBS += -lboost_system
