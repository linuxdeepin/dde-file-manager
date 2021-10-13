/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     wangchunlin<wangchunlin@uniontech.com>
 *
 * Maintainer: wangchunlin<wangchunlin@uniontech.com>
 *             xinglinkun<xinglinkun@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

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
