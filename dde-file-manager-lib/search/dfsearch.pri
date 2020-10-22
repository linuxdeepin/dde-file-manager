# Copyright (C) 2019 ~ 2020 Uniontech Technology Co., Ltd.

#Author:     hujianzhong <hujianhzong@uniontech.com>

#Maintainer: hujianzhong <hujianhzong@uniontech.com>

#This program is free software: you can redistribute it and/or modify
#it under the terms of the GNU General Public License as published by
#the Free Software Foundation, either version 3 of the License, or
#any later version.

#This program is distributed in the hope that it will be useful,
#but WITHOUT ANY WARRANTY; without even the implied warranty of
#MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#GNU General Public License for more details.
#You should have received a copy of the GNU General Public License
#along with this program.  If not, see <http://www.gnu.org/licenses/>.

#This program is the full text search at dde-file-manager.
#-------------------------------------------------
#
# Project created by QtCreator 2020-05-12T16:00:00
#这个搜索引擎是利用C语言和GTK代码编写，文件搜索速度相当快，是移植的开源代码fsearch的核心部分
#-------------------------------------------------
HEADERS += \ 
    $$PWD/dfsearch.h \
    $$PWD/fsearch/array.h \
    $$PWD/fsearch/btree.h \
    $$PWD/fsearch/database_search.h \
    $$PWD/fsearch/database.h \
    $$PWD/fsearch/fsearch_config.h \
    $$PWD/fsearch/fsearch_limits.h \
    $$PWD/fsearch/fsearch_thread_pool.h \
    $$PWD/fsearch/fsearch.h \
    $$PWD/fsearch/query.h \
    $$PWD/fsearch/string_utils.h \
    $$PWD/fsearch/utf8.h

SOURCES += \ 
    $$PWD/dfsearch.cpp \
    $$PWD/fsearch/array.c \
    $$PWD/fsearch/btree.c \
    $$PWD/fsearch/database_search.c \
    $$PWD/fsearch/database.c \
    $$PWD/fsearch/fsearch_config.c \
    $$PWD/fsearch/fsearch_thread_pool.c \
    $$PWD/fsearch/fsearch.c \
    $$PWD/fsearch/query.c \
    $$PWD/fsearch/string_utils.c

CONFIG += link_pkgconfig

INCLUDEPATH += $$PWD/fsearch
