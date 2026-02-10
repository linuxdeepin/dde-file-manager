// SPDX-FileCopyrightText: 2024 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef RECENTDAEMON_H
#define RECENTDAEMON_H

#include "serverplugin_recentmanager_global.h"

#include <dfm-framework/dpf.h>

#include <QDBusConnection>

class RecentManagerDBus;

SERVERRECENTMANAGER_BEGIN_NAMESPACE

class RecentDaemon : public DPF_NAMESPACE::Plugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.deepin.plugin.daemon" FILE "recentdaemon.json")

public:
    virtual void initialize() override;
    virtual bool start() override;
    virtual void stop() override;

private:
    QScopedPointer<RecentManagerDBus> recentManager;
};

SERVERRECENTMANAGER_END_NAMESPACE

#endif   // RECENTDAEMON_H
