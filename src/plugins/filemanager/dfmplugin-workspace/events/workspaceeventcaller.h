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

DPWORKSPACE_BEGIN_NAMESPACE
DFMBASE_USE_NAMESPACE

class WorkspaceEventCaller
{
    WorkspaceEventCaller() = delete;

public:
    static void sendOpenWindow(const QList<QUrl> &urls);
    static void sendChangeCurrentUrl(const quint64 windowId, const QUrl &url);

    static void sendTabAdded(const quint64 windowID);
    static void sendTabChanged(const quint64 windowID, const int index);
    static void sendTabMoved(const quint64 windowID, const int from, const int to);
    static void sendTabRemoved(const quint64 windowID, const int index);
    static void sendOpenFiles(const quint64 windowID, const QList<QUrl> &urls);
    static void sendMoveToTrash(quint64 windowID, const QList<QUrl> &urls, const AbstractJobHandler::JobFlags flags = AbstractJobHandler::JobFlag::kNoHint);
    static void sendNewFolder(quint64 windowID, const QUrl &url);
    static void sendDeletes(quint64 windowID, const QList<QUrl> &urls, const AbstractJobHandler::JobFlags flags = AbstractJobHandler::JobFlag::kNoHint);
    static void sendRenameFile(quint64 windowID, const QUrl &oldUrl, const QUrl &newUrl);
    static void sendWriteToClipboard(const quint64 windowId, const ClipBoard::ClipboardAction action, const QList<QUrl> &urls);
    static void sendCopyFiles(const quint64 windowId, const QList<QUrl> &sourceUrls, const QUrl &target, const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags flags);
    static void sendCutFiles(const quint64 windowId, const QList<QUrl> &sourceUrls, const QUrl &target, const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags flags);
};

DPWORKSPACE_END_NAMESPACE

#endif   // WORKSPACEEVENTCALLER_H
