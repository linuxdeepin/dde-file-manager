# SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

cmake_minimum_required(VERSION 3.10)

project(ds-panel-file-manager)

find_package(DDEShell REQUIRED)
find_package(Qt6 COMPONENTS DBus REQUIRED)

add_library(ds-panel-file-manager SHARED
    filemanagerplugin.cpp
    filemanagerplugin.h
    directorymodel.cpp
    directorymodel.h
)

target_link_libraries(ds-panel-file-manager PRIVATE
    Dde::Shell
    Qt6::DBus
)

ds_install_package(PACKAGE org.deepin.ds.filemanager TARGET ds-panel-file-manager)
