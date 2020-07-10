/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#pragma once

#include <QObject>
#include "../global/singleton.h"

class QScreen;
class DBusDisplay;
class DBusAppearance;
class Display: public QObject, public Singleton<Display>
{
    Q_OBJECT

    friend class Singleton<Display>;
public:
    explicit Display(QObject *parent = nullptr);

    QRect primaryRect();
    QString primaryName();

    QScreen *primaryScreen();

    QStringList monitorObjectPaths() const;

    double getScaleFactor();
signals:
    void primaryScreenChanged(QScreen *screen);
    void primaryChanged();
    void sigDisplayModeChanged();
    void sigMonitorsChanged(QScreen *screen);

private:
    DBusDisplay *m_display = nullptr;
    DBusAppearance *m_appearance = nullptr;
};

class DBusDock;
class DockIns: public QObject, public Singleton<DBusDock>
{
    Q_OBJECT

    friend class Singleton<DBusDock>;
public:
    explicit DockIns(QObject *parent = nullptr);

private:
    DBusDock *m_dock = nullptr;
};
