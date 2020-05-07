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

#define USINGOLD 0

class QDBusConnection;
class CanvasGridView;
class DesktopPrivate;
class Desktop : public QObject, public Singleton<Desktop>
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", DesktopServiceInterface)
public:
    void loadData();
    void loadView();

    void showZoneSettings();

    void initDebugDBus(QDBusConnection &conn);
public slots:
    void EnableUIDebug(bool enable);
    void SetVisible(int screenNum,bool);
    void FixGeometry(int screenNum);
    void Reset();
    void ShowInfo();
    void ShowWallpaperChooser(const QString &screen = QString());
    void ShowScreensaverChooser(const QString &screen = QString());
protected:
    void showWallpaperSettings(QString name, int mode = 0);
#if USINGOLD
#ifdef QT_DEBUG
    void logAllScreenLabel();
    void logScreenLabel(int index);
    void mapLabelScreen(int labelIndex, int screenIndex);
#endif // QT_DEBUG
#endif
private:
    explicit Desktop();
    ~Desktop();
#if USINGOLD
    void onBackgroundEnableChanged();
    void onBackgroundGeometryChanged(QWidget *l);
#endif
    friend class Singleton<Desktop>;
    Q_DISABLE_COPY(Desktop)

    QScopedPointer<DesktopPrivate> d;
};

#endif // DESKTOP_H
