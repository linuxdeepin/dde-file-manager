/*
 * Copyright (C) 2021 ~ 2022 Uniontech Software Technology Co., Ltd.
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
#ifndef WORKSPACE_H
#define WORKSPACE_H

#include "dfmplugin_workspace_global.h"
#include <dfm-framework/framework.h>
#include "dfm-framework/dpf.h"

DPWORKSPACE_BEGIN_NAMESPACE

class Workspace : public dpf::Plugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.deepin.plugin.filemanager" FILE "workspace.json")

    DPF_EVENT_NAMESPACE(DPWORKSPACE_NAMESPACE)
    DPF_EVENT_REG_HOOK(hook_FetchCustomColumnRoles)
    DPF_EVENT_REG_HOOK(hook_FetchCustomRoleDisplayName)
    DPF_EVENT_REG_HOOK(hook_FetchCustomRoleData)
    DPF_EVENT_REG_HOOK(hook_CheckDragDropAction)

public:
    virtual void initialize() override;
    virtual bool start() override;
    virtual ShutdownFlag stop() override;

private slots:
    void onWindowOpened(quint64 windId);
    void onWindowClosed(quint64 winId);
};

DPWORKSPACE_END_NAMESPACE

#endif   // WORKSPACE_H
