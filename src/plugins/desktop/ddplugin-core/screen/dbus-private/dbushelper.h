// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DOCKHELPER_H
#define DOCKHELPER_H

#include "ddplugin_core_global.h"

#ifdef COMPILE_ON_V2X
#include "dbusdock1.h"
#include "dbusdisplay1.h"
//#include "dbusmonitor1.h"
#else
#include "dbusdock.h"
//#include "dbusmonitor.h"
#include "dbusdisplay.h"
#endif

#include <QObject>

DDPCORE_BEGIN_NAMESPACE

class DBusHelper : public QObject
{
    Q_OBJECT
public:
    static DBusHelper *ins();
    static bool isDockEnable();
    static bool isDisplayEnable();
    DBusDock *dock() const;
    DBusDisplay *display() const;
#if 0
    DBusMonitor *createMonitor(const QString &path);
#endif
private:
    explicit DBusHelper(QObject *parent = nullptr);
    DBusDock *m_dock = nullptr;
    DBusDisplay *m_display = nullptr;
};

DDPCORE_END_NAMESPACE

#define DockInfoIns ddplugin_core::DBusHelper::ins()->dock()
#define DisplayInfoIns ddplugin_core::DBusHelper::ins()->display()

#endif // DOCKHELPER_H
