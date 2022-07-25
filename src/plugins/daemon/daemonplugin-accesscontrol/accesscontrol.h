/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangsheng<zhangsheng@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             xushitong<xushitong@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef ACCESSCONTROL_H
#define ACCESSCONTROL_H

#include "daemonplugin_accesscontrol_global.h"

#include "accesscontroldbus.h"

#include <dfm-framework/dpf.h>

#include <dfm-io/local/dlocalwatcher.h>

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
    QScopedPointer<DFMIO::DLocalWatcher> watcher;
};

DAEMONPAC_END_NAMESPACE

#endif   // ACCESSCONTROL_H
