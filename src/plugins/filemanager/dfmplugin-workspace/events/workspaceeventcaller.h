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
#ifndef WORKSPACEEVENTCALLER_H
#define WORKSPACEEVENTCALLER_H

#include "dfmplugin_workspace_global.h"
#include "dfm-base/interfaces/abstractjobhandler.h"
#include "dfm-base/dfm_base_global.h"
#include "dfm-base/utils/clipboard.h"

#include <QObject>

DFMBASE_USE_NAMESPACE
DPWORKSPACE_BEGIN_NAMESPACE

class WorkspaceEventCaller
{
    WorkspaceEventCaller() = delete;

public:
    static void sendOpenWindow(const QList<QUrl> &urls);
    static void sendChangeCurrentUrl(const quint64 windowId, const QUrl &url);
    static void sendOpenAsAdmin(const QUrl &url);

    static void sendTabAdded(const quint64 windowID);
    static void sendTabChanged(const quint64 windowID, const int index);
    static void sendTabMoved(const quint64 windowID, const int from, const int to);
    static void sendTabRemoved(const quint64 windowID, const int index);
    static void sendShowCustomTopWidget(const quint64 windowId, const QString &scheme, bool visible);
    static void sendSetSelectDetailFileUrl(const quint64 windowId, const QUrl &url);
};

DPWORKSPACE_END_NAMESPACE

#endif   // WORKSPACEEVENTCALLER_H
