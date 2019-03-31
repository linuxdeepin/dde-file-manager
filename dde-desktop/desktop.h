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
class QStyleOptionViewItem;
class DesktopPrivate;
class Desktop : public QObject, public Singleton<Desktop>
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", DesktopServiceInterface)
public:
    void loadData();
    void loadView();

    void showWallpaperSettings();
    void showZoneSettings();

    void initDebugDBus(QDBusConnection &conn);

    CanvasGridView *getView();

public slots:
    void Show();

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
