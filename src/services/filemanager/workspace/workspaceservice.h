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
#include "dfm_global_defines.h"

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

    static WorkspaceService *service();

    void addScheme(const QString &scheme);
    bool schemeViewIsFileView(const QString &scheme);
    bool tabAddable(const quint64 windowID);
    void addCustomTopWidget(const Workspace::CustomTopWidgetInfo &info);
    bool getCustomTopWidgetVisible(const quint64 windowID, const QString &scheme);
    void setFileViewFilterData(const quint64 windowID, const QUrl &url, const QVariant &data);
    void setFileViewFilterCallback(const quint64 windowID, const QUrl &url, const Workspace::FileViewFilterCallback callback);
    void setWorkspaceMenuScene(const QString &scheme, const QString &scene);
    void setDefaultViewMode(const QString &scheme, const DFMBASE_NAMESPACE::Global::ViewMode mode);
    DFMBASE_NAMESPACE::Global::ViewMode currentViewMode(const quint64 windowID);
    DFMBASE_NAMESPACE::Global::ViewMode getDefaultViewMode(const QString &scheme);
    bool registerFileViewRoutePrehandle(const QString &scheme, const Workspace::FileViewRoutePrehaldler &prehandler);

private:
    explicit WorkspaceService(QObject *parent = nullptr);
    virtual ~WorkspaceService() override;

    QScopedPointer<WorkspaceServicePrivate> d;
};

DSB_FM_END_NAMESPACE

#endif   // WORKSPACESERVICE_H
