// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "coreeventscaller.h"
#include "utils/corehelper.h"

#include <dfm-base/dfm_event_defines.h>
#include <dfm-base/widgets/filemanagerwindowsmanager.h>

#include <dfm-framework/event/event.h>

#include <QUrl>

DFMBASE_USE_NAMESPACE
using namespace filedialog_core;

void CoreEventsCaller::sendViewMode(QWidget *sender, DFMBASE_NAMESPACE::Global::ViewMode mode)
{
    quint64 id = FMWindowsIns.findWindowId(sender);
    Q_ASSERT(id > 0);

    dpfSignalDispatcher->publish(DFMBASE_NAMESPACE::GlobalEventType::kSwitchViewMode, id, int(mode));
}

void CoreEventsCaller::sendSelectFiles(quint64 windowId, const QList<QUrl> &files)
{
    dpfSlotChannel->push("dfmplugin_workspace", "slot_View_SelectFiles", windowId, files);
}

void CoreEventsCaller::setSidebarItemVisible(const QUrl &url, bool visible)
{
    dpfSlotChannel->push("dfmplugin_sidebar", "slot_Item_Hidden",
                         url, visible);
}

void CoreEventsCaller::setSelectionMode(QWidget *sender, const QAbstractItemView::SelectionMode mode)
{
    quint64 id = FMWindowsIns.findWindowId(sender);
    Q_ASSERT(id > 0);
    auto func = [id, mode]() {
        dpfSlotChannel->push("dfmplugin_workspace", "slot_View_SetSelectionMode", id, mode);
    };
    CoreHelper::delayInvokeProxy(func, id, sender);
}

void CoreEventsCaller::setEnabledSelectionModes(QWidget *sender, const QList<QAbstractItemView::SelectionMode> &modes)
{
    quint64 id = FMWindowsIns.findWindowId(sender);
    Q_ASSERT(id > 0);

    auto func = [id, modes] {
        dpfSlotChannel->push("dfmplugin_workspace", "slot_View_SetEnabledSelectionModes", id, modes);
    };
    CoreHelper::delayInvokeProxy(func, id, sender);
}

void CoreEventsCaller::setMenuDisbaled()
{
    dpfSlotChannel->push("dfmplugin_sidebar", "slot_ContextMenu_SetEnable", false);
    dpfSlotChannel->push("dfmplugin_computer", "slot_ContextMenu_SetEnable", false);
    dpfSlotChannel->push("dfmplugin_titlebar", "slot_NewWindowAndTab_SetEnable", false);
}

QList<QUrl> CoreEventsCaller::sendGetSelectedFiles(const quint64 windowID)
{
    return dpfSlotChannel->push("dfmplugin_workspace", "slot_View_GetSelectedUrls", windowID).value<QList<QUrl>>();
}
