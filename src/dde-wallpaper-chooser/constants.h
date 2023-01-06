// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CONSTANTS_H
#define CONSTANTS_H

static const int FrameHeight = 130;
static const int ListHeight = 100;
static const int HeaderSwitcherHeight = 45;

static const int ItemCellWidth = 172;
static const int ItemCellHeight = ListHeight;
static const int ItemWidth = ItemCellWidth;
static const int ItemHeight = ListHeight;

static const QString AppearanceServ = "com.deepin.daemon.Appearance";
static const QString AppearancePath = "/com/deepin/daemon/Appearance";

static const QString DeepinWMServ = "com.deepin.wm";
static const QString DeepinWMPath = "/com/deepin/wm";

static const QString MouseAreaServ = "com.deepin.api.XMouseArea";
static const QString MouseAreaPath = "/com/deepin/api/XMouseArea";

static const QByteArray WallpaperSchemaId = "com.deepin.wrap.gnome.desktop.background";
static const QByteArray WallpaperPath = "/com/deepin/wrap/gnome/desktop/background/";
static const QString WallpaperKey = "pictureUri";

#endif // CONSTANTS_H
