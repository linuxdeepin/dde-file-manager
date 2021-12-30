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
#include "detailspacehelper.h"
#include "views/detailspacewidget.h"

#include "services/filemanager/windows/windowsservice.h"

#include <dfm-framework/framework.h>

DPDETAILSPACE_USE_NAMESPACE
DSB_FM_USE_NAMESPACE

QMap<quint64, DetailSpaceWidget *> DetailSpaceHelper::kDetailSpaceMap {};

DetailSpaceWidget *DetailSpaceHelper::findDetailSpaceByWindowId(quint64 windowId)
{
    if (!kDetailSpaceMap.contains(windowId))
        return nullptr;

    return kDetailSpaceMap[windowId];
}

void DetailSpaceHelper::addDetailSpace(quint64 windowId, DetailSpaceWidget *titleBar)
{
    QMutexLocker locker(&DetailSpaceHelper::mutex());
    if (!kDetailSpaceMap.contains(windowId))
        kDetailSpaceMap.insert(windowId, titleBar);
}

void DetailSpaceHelper::removeDetailSpace(quint64 windowId)
{
    QMutexLocker locker(&DetailSpaceHelper::mutex());
    if (kDetailSpaceMap.contains(windowId))
        kDetailSpaceMap.remove(windowId);
}

void DetailSpaceHelper::showDetailView(quint64 windowId, bool checked)
{
    DetailSpaceWidget *w = findDetailSpaceByWindowId(windowId);

    // create new detail widget in window and
    if (!w) {
        auto &ctx = dpfInstance.serviceContext();
        auto windowService = ctx.service<WindowsService>(WindowsService::name());
        auto window = windowService->findWindowById(windowId);
        w = new DetailSpaceWidget;
        window->installDetailView(w);
        addDetailSpace(windowId, w);
    }

    w->setVisible(checked);
}

QMutex &DetailSpaceHelper::mutex()
{
    static QMutex m;
    return m;
}
