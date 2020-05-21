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
    $$PWD/array.h \
    $$PWD/btree.h \
    $$PWD/database_search.h \
    $$PWD/database.h \
    $$PWD/fsearch_config.h \
    $$PWD/fsearch_limits.h \
    $$PWD/fsearch_thread_pool.h \
    $$PWD/fsearch.h \
    $$PWD/myfsearch.h \
    $$PWD/query.h \
    $$PWD/string_utils.h \
    $$PWD/utf8.h

SOURCES += \ 
    $$PWD/array.c \
    $$PWD/btree.c \
    $$PWD/database_search.c \
    $$PWD/database.c \
    $$PWD/fsearch_config.c \
    $$PWD/fsearch_thread_pool.c \
    $$PWD/fsearch.c \
    $$PWD/myfsearch.c \
    $$PWD/query.c \
    $$PWD/string_utils.c

CONFIG += link_pkgconfig
