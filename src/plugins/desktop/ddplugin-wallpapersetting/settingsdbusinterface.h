// SPDX-FileCopyrightText: 2023 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SETTINGSDBUSINTERFACE_H
#define SETTINGSDBUSINTERFACE_H

#include "ddplugin_wallpapersetting_global.h"

#include <QObject>
#include <QDBusContext>

namespace ddplugin_wallpapersetting {
class EventHandle;
class SettingsDBusInterface : public QObject, public QDBusContext
{
    Q_OBJECT
public:
    explicit SettingsDBusInterface(EventHandle *parent = nullptr);

signals:

public slots:
    void ShowWallpaperChooser(const QString &screen = "");
    void ShowScreensaverChooser(const QString &screen = "");

private:
    EventHandle *handle;
};

}

#endif   // SETTINGSDBUSINTERFACE_H
