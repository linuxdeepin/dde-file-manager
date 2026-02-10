// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
