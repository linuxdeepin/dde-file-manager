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
#include "workspaceservice.h"
#include "private/workspaceservice_p.h"

#include "dfm-base/utils/universalutils.h"

DSB_FM_BEGIN_NAMESPACE

namespace Workspace {

// custom event type
namespace EventType {
const int kTabAdded = DFMBASE_NAMESPACE::UniversalUtils::registerEventType();
const int kTabChanged = DFMBASE_NAMESPACE::UniversalUtils::registerEventType();
const int kTabMoved = DFMBASE_NAMESPACE::UniversalUtils::registerEventType();
const int kTabRemoved = DFMBASE_NAMESPACE::UniversalUtils::registerEventType();
const int kCloseTabs = DFMBASE_NAMESPACE::UniversalUtils::registerEventType();
const int kShowCustomTopWidget = DFMBASE_NAMESPACE::UniversalUtils::registerEventType();
const int kPaintListItem = DFMBASE_NAMESPACE::UniversalUtils::registerEventType();
const int kPaintIconItem = DFMBASE_NAMESPACE::UniversalUtils::registerEventType();
const int kCheckDragDropAction = DFMBASE_NAMESPACE::UniversalUtils::registerEventType();
};   // namespace EventType

namespace MenuScene {
const char *const kWorkspaceMenu = "workspace-menu";
};   // namespace MenuScene

}   // namespace Sidebar

DSB_FM_END_NAMESPACE

DSB_FM_USE_NAMESPACE

/*!
 * \class WorkspaceServicePrivate
 * \brief
 */

WorkspaceServicePrivate::WorkspaceServicePrivate(WorkspaceService *serv)
    : QObject(nullptr), service(serv)
{
}

/*!
 * \class WorkspaceService
 * \brief
 */

WorkspaceService::WorkspaceService(QObject *parent)
    : dpf::PluginService(parent),
      dpf::AutoServiceRegister<WorkspaceService>(),
      d(new WorkspaceServicePrivate(this))
{
}

WorkspaceService::~WorkspaceService()
{
}

WorkspaceService *WorkspaceService::service()
{
    auto &ctx = dpfInstance.serviceContext();
    static std::once_flag onceFlag;
    std::call_once(onceFlag, [&ctx]() {
        if (!ctx.load(name()))
            abort();
    });

    return ctx.service<WorkspaceService>(name());
}

void WorkspaceService::addScheme(const QString &scheme)
{
    dpfInstance.eventUnicast().push(DSB_FUNC_NAME, scheme);
}

bool WorkspaceService::tabAddable(const quint64 windowID)
{
    return dpfInstance.eventUnicast().push(DSB_FUNC_NAME, windowID).toBool();
}

void WorkspaceService::addCustomTopWidget(const Workspace::CustomTopWidgetInfo &info)
{
    dpfInstance.eventUnicast().push(DSB_FUNC_NAME, info);
}

bool WorkspaceService::getCustomTopWidgetVisible(const quint64 windowID, const QString &scheme)
{
    return dpfInstance.eventUnicast().push(DSB_FUNC_NAME, windowID, scheme).toBool();
}

void WorkspaceService::setFileViewFilterData(const quint64 windowID, const QUrl &url, const QVariant &data)
{
    dpfInstance.eventUnicast().push(DSB_FUNC_NAME, windowID, url, data);
}

void WorkspaceService::setFileViewFilterCallback(const quint64 windowID, const QUrl &url, const Workspace::FileViewFilterCallback callback)
{
    dpfInstance.eventUnicast().push(DSB_FUNC_NAME, windowID, url, callback);
}

void WorkspaceService::setWorkspaceMenuScene(const QString &scheme, const QString &scene)
{
    dpfInstance.eventUnicast().push(DSB_FUNC_NAME, scheme, scene);
}

void WorkspaceService::setDefaultViewMode(const QString &scheme, const dfmbase::Global::ViewMode mode)
{
    dpfInstance.eventUnicast().push(DSB_FUNC_NAME, scheme, mode);
}

DFMBASE_NAMESPACE::Global::ViewMode WorkspaceService::getDefaultViewMode(const QString &scheme)
{
    return dpfInstance.eventUnicast().push(DSB_FUNC_NAME, scheme).value<DFMBASE_NAMESPACE::Global::ViewMode>();
}

bool WorkspaceService::registerFileViewRoutePrehandle(const QString &scheme, const Workspace::FileViewRoutePrehaldler &prehandler)
{
    return dpfInstance.eventUnicast().push(DSB_FUNC_NAME, scheme, prehandler).toBool();
}
