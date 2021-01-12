/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#ifndef DESKTOP_H
#define DESKTOP_H

#include <QObject>
#include <QScopedPointer>

#include "global/singleton.h"

#define DesktopServiceName          "com.deepin.dde.desktop"
#define DesktopServicePath          "/com/deepin/dde/desktop"
#define DesktopServiceInterface     "com.deepin.dde.desktop"

class QDBusConnection;
class CanvasGridView;
class DesktopPrivate;
class Desktop : public QObject, public Singleton<Desktop>
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", DesktopServiceInterface)
public:
    void preInit();
    void loadData();
    void loadView();
    void showZoneSettings();

public slots:
    void ShowWallpaperChooser(const QString &screen = QString());
    void ShowScreensaverChooser(const QString &screen = QString());

    Q_SCRIPTABLE void EnableUIDebug(bool enable);
    Q_SCRIPTABLE void SetVisible(int screenNum,bool);
    Q_SCRIPTABLE void FixGeometry(int screenNum);
    Q_SCRIPTABLE void Reset();
    Q_SCRIPTABLE void PrintInfo();
    Q_SCRIPTABLE void Refresh();
    Q_SCRIPTABLE QList<int> GetIconSize();
protected:
    void showWallpaperSettings(QString name, int mode = 0);
private:
    explicit Desktop();
    ~Desktop();
    friend class Singleton<Desktop>;
    Q_DISABLE_COPY(Desktop)

    QScopedPointer<DesktopPrivate> d;
};

#endif // DESKTOP_H
