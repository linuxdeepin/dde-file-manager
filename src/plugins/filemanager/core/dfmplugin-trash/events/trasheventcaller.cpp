// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "trasheventcaller.h"
#include "utils/trashhelper.h"

#include <dfm-base/dfm_event_defines.h>
#include <dfm-base/utils/dialogmanager.h>
#include <dfm-framework/dpf.h>

#include <QUrl>

using namespace dfmplugin_trash;
DFMBASE_USE_NAMESPACE

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
    dpfSlotChannel->push("dfmplugin_propertydialog", "slot_PropertyDialog_Show", QList<QUrl>() << url, QVariantHash());
}

void TrashEventCaller::sendShowEmptyTrash(quint64 winId, bool visible)
{
    dpfSlotChannel->push("dfmplugin_workspace", "slot_ShowCustomTopWidget", winId, TrashHelper::scheme(), visible);
}

bool TrashEventCaller::sendCheckTabAddable(quint64 windowId)
{
    return dpfSlotChannel->push("dfmplugin_titlebar", "slot_Tab_Addable", windowId).toBool();
}
