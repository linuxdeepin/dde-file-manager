/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     yanghao<yanghao@uniontech.com>
 *
 * Maintainer: liuyangming<liuyangming@uniontech.com>
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
#include "trasheventcaller.h"
#include "utils/trashhelper.h"

#include "services/filemanager/workspace/workspace_defines.h"

#include "dfm-base/dfm_event_defines.h"
#include "dfm-base/utils/dialogmanager.h"
#include <dfm-framework/framework.h>

#include <QUrl>

using namespace dfmplugin_trash;
DFMBASE_USE_NAMESPACE
DSB_FM_USE_NAMESPACE

void TrashEventCaller::sendOpenWindow(const QUrl &url)
{
    dpfSignalDispatcher->publish(GlobalEventType::kOpenNewWindow, url);
}

void TrashEventCaller::sendOpenTab(quint64 windowId, const QUrl &url)
{
    dpfSignalDispatcher->publish(GlobalEventType::kOpenNewTab, windowId, url);
}

void TrashEventCaller::sendOpenFiles(const quint64 windowID, const QList<QUrl> &urls)
{
    dpfSignalDispatcher->publish(GlobalEventType::kOpenFiles, windowID, urls);
}

void TrashEventCaller::sendEmptyTrash(const quint64 windowID, const QList<QUrl> &urls)
{
    dpfSignalDispatcher->publish(GlobalEventType::kCleanTrash, windowID, urls, AbstractJobHandler::DeleteDialogNoticeType::kEmptyTrash, nullptr);
}

void TrashEventCaller::sendTrashPropertyDialog(const QUrl &url)
{
    dpfSlotChannel->push("dfmplugin_propertydialog", "slot_PropertyDialog_Show", QList<QUrl>() << url);
}

void TrashEventCaller::sendShowEmptyTrash(quint64 winId, bool visible)
{
    dpfSignalDispatcher->publish(Workspace::EventType::kShowCustomTopWidget, winId, TrashHelper::scheme(), visible);
}
