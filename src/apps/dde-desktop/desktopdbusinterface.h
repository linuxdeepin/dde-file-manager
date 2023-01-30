/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangyu<zhangyub@uniontech.com>
 *
 * Maintainer: zhangyu<zhangyub@uniontech.com>
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
#ifndef DESKTOPDBUSINTERFACE_H
#define DESKTOPDBUSINTERFACE_H

#include <dfm-framework/dpf.h>

#include <QDBusContext>
#include <QObject>

namespace dde_desktop {

inline constexpr char kDesktopServiceName[] { "com.deepin.dde.desktop" };
inline constexpr char kDesktopServicePath[] { "/com/deepin/dde/desktop" };
inline constexpr char kDesktopServiceInterface[] { "com.deepin.dde.desktop" };

class DesktopDBusInterface : public QObject, public QDBusContext
{
    Q_OBJECT
public:
    explicit DesktopDBusInterface(QObject *parent = nullptr);
signals:

public slots:
    // the listed method is to compatible with the old.
    void Refresh(bool silent = true);
    void ShowWallpaperChooser(const QString &screen = "");
    void ShowScreensaverChooser(const QString &screen = "");
};

}
#endif   // DESKTOPDBUSINTERFACE_H
