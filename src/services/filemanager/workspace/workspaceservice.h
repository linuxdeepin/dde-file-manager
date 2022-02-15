/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
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
#ifndef WORKSPACESERVICE_H
#define WORKSPACESERVICE_H

#include "workspace_defines.h"

#include <dfm-framework/framework.h>

DSB_FM_BEGIN_NAMESPACE

class WorkspaceServicePrivate;
class WorkspaceService final : public dpf::PluginService, dpf::AutoServiceRegister<WorkspaceService>
{
    Q_OBJECT
    Q_DISABLE_COPY(WorkspaceService)
    friend class dpf::QtClassFactory<dpf::PluginService>;

public:
    static QString name()
    {
        return "org.deepin.service.WorkspaceService";
    }

    void addScheme(const QString &scheme);
    bool tabAddable(const quint64 windowID);
    void addCustomTopWidget(const Workspace::CustomTopWidgetInfo &info);

private:
    explicit WorkspaceService(QObject *parent = nullptr);
    virtual ~WorkspaceService() override;

    QScopedPointer<WorkspaceServicePrivate> d;
};

DSB_FM_END_NAMESPACE

#endif   // WORKSPACESERVICE_H
