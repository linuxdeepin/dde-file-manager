/**
 * Copyright (C) 2015 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#ifndef ZONEMAINWINDOWWITHCLICK_H
#define ZONEMAINWINDOWWITHCLICK_H

#include <QWidget>
#include <QMouseEvent>
#include <QKeyEvent>

#include "dbus/dbuszone.h"
#include <QLabel>
#include <DVideoWidget>

DWIDGET_USE_NAMESPACE

class ZoneMainWindow : public QWidget
{
    Q_OBJECT
public:
    explicit ZoneMainWindow(QWidget *parent = nullptr);
    ~ZoneMainWindow();

    void mousePressEvent(QMouseEvent *);
    void keyPressEvent(QKeyEvent *);

signals:
    void finished();

private slots:
    void onDemoVideo();

private:
    void paintEvent(QPaintEvent *e) override;

    QStringList m_ButtonNames;
    QStringList m_ActionStrs;

    ZoneInterface *m_dbusZoneInter;

    // all the actions' dbus-about string
    const QString ALL_WINDOWS_STR = "dbus-send --session --dest=com.deepin.wm --print-reply /com/deepin/wm com.deepin.wm.PerformAction int32:6";
    const QString LAUNCHER_STR = "dbus-send --print-reply --dest=com.deepin.dde.Launcher /com/deepin/dde/Launcher com.deepin.dde.Launcher.Toggle";
    const QString SHOW_DESKTOP_STR = "/usr/lib/deepin-daemon/desktop-toggle";
    const QString NONE_STR = "";
    const QString FAST_SCREEN_OFF = "sh -c \"sleep 0.5 && xset dpms force off\"";
    const QString CLOSE_MAX_WINDOW_STR = "!wm:close";

    // MAIN_ITEM_TOP_MARGIN is aimed to steer clear of the fade-zone of mouseEvent of mainWindow Item.
    const int MAIN_ITEM_TOP_MARGIN = 30;
    DVideoWidget *m_videoWidget;
};

#endif // ZONEMAINWINDOWWITHCLICK_H
