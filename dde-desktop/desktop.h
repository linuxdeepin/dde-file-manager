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

    void showWallpaperSettings(int mode = 0);
    void showZoneSettings();

    void initDebugDBus(QDBusConnection &conn);

    CanvasGridView *getView();

public slots:
    void ShowWallpaperChooser();
    void ShowScreensaverChooser();

    Q_SCRIPTABLE void Show();
    Q_SCRIPTABLE void Refresh();
    Q_SCRIPTABLE void logAllScreenLabel();
    Q_SCRIPTABLE void logScreenLabel(int index);
    Q_SCRIPTABLE void mapLabelScreen(int labelIndex, int screenIndex);
    Q_SCRIPTABLE QList<int> GetIconSize();

private:
    explicit Desktop();
    ~Desktop();

    void onBackgroundEnableChanged();
    void onBackgroundGeometryChanged(QWidget *l);

    friend class Singleton<Desktop>;
    Q_DISABLE_COPY(Desktop)

    QScopedPointer<DesktopPrivate> d;
};

#endif // DESKTOP_H
