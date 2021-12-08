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
#这个搜索引擎是利用C++语言，文件搜索速度相当快，是移植的开源代码doctotext的核心部分
#-------------------------------------------------

QT += widgets
CONFIG += c++11 link_pkgconfig
PKGCONFIG += liblucene++ liblucene++-contrib docparser

HEADERS += \
    $$PWD/fulltextsearch.h \
    $$PWD/chinesetokenizer.h \
    $$PWD/chineseanalyzer.h

SOURCES += \
    $$PWD/fulltextsearch.cpp \
    $$PWD/chinesetokenizer.cpp \
    $$PWD/chineseanalyzer.cpp

INCLUDEPATH += $$PWD
LIBS += -lboost_system
