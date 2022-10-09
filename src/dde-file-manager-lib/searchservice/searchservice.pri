include(../../common/common.pri)

HEADERS += \
    $$PWD/maincontroller/task/taskcommander.h \
    $$PWD/maincontroller/task/taskcommander_p.h \
    $$PWD/maincontroller/maincontroller.h \
    $$PWD/private/searchservice_p.h \
    $$PWD/searcher/fulltext/chineseanalyzer.h \
    $$PWD/searcher/fulltext/chinesetokenizer.h \
    $$PWD/searcher/fulltext/fulltextsearcher.h \
    $$PWD/searcher/fulltext/fulltextsearcher_p.h \
    $$PWD/searcher/iterator/iteratorsearcher.h \
    $$PWD/searcher/abstractsearcher.h \
    $$PWD/utils/searchhelper.h \
    $$PWD/searchservice.h \
    $$PWD/searcher/fsearch/fssearcher.h \
#    -----------fsearch source---------------
    $$PWD/../../../3rdparty/fsearch/query.h \
    $$PWD/../../../3rdparty/fsearch/array.h \
    $$PWD/../../../3rdparty/fsearch/btree.h \
    $$PWD/../../../3rdparty/fsearch/database_search.h \
    $$PWD/../../../3rdparty/fsearch/database.h \
    $$PWD/../../../3rdparty/fsearch/fsearch_config.h \
    $$PWD/../../../3rdparty/fsearch/fsearch_limits.h \
    $$PWD/../../../3rdparty/fsearch/fsearch_thread_pool.h \
    $$PWD/../../../3rdparty/fsearch/fsearch.h \
    $$PWD/../../../3rdparty/fsearch/string_utils.h \
    $$PWD/../../../3rdparty/fsearch/utf8.h
#    -----------fsearch source---------------

SOURCES += \
    $$PWD/maincontroller/task/taskcommander.cpp \
    $$PWD/maincontroller/maincontroller.cpp \
    $$PWD/searcher/fulltext/chineseanalyzer.cpp \
    $$PWD/searcher/fulltext/chinesetokenizer.cpp \
    $$PWD/searcher/fulltext/fulltextsearcher.cpp \
    $$PWD/searcher/iterator/iteratorsearcher.cpp \
    $$PWD/searcher/abstractsearcher.cpp \
    $$PWD/utils/searchhelper.cpp \
    $$PWD/searchservice.cpp \
    $$PWD/searcher/fsearch/fssearcher.cpp \
#    -----------fsearch source---------------
    $$PWD/../../../3rdparty/fsearch/array.c \
    $$PWD/../../../3rdparty/fsearch/btree.c \
    $$PWD/../../../3rdparty/fsearch/database_search.c \
    $$PWD/../../../3rdparty/fsearch/database.c \
    $$PWD/../../../3rdparty/fsearch/fsearch_config.c \
    $$PWD/../../../3rdparty/fsearch/fsearch_thread_pool.c \
    $$PWD/../../../3rdparty/fsearch/fsearch.c \
    $$PWD/../../../3rdparty/fsearch/query.c \
    $$PWD/../../../3rdparty/fsearch/string_utils.c
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
