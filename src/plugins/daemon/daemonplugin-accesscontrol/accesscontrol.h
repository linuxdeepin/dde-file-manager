// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ACCESSCONTROL_H
#define ACCESSCONTROL_H

#include "daemonplugin_accesscontrol_global.h"

#include "accesscontroldbus.h"

#include <dfm-framework/dpf.h>

#include <dfm-io/dwatcher.h>

DAEMONPAC_BEGIN_NAMESPACE

class AccessControl : public DPF_NAMESPACE::Plugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.deepin.plugin.daemon" FILE "accesscontrol.json")

public:
    virtual bool start() override;

private:
    bool isDaemonServiceRegistered();
    void initDBusInterce();
    void initConnect();

private slots:
    void onFileCreatedInHomePath();

private:
    QScopedPointer<AccessControlDBus> accessControlManager;
    QScopedPointer<DFMIO::DWatcher> watcher;
};

DAEMONPAC_END_NAMESPACE

#endif   // ACCESSCONTROL_H
