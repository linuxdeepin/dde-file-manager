// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef COREEVENTSCALLER_H
#define COREEVENTSCALLER_H

#include "filedialogplugin_core_global.h"

#include <dfm-base/dfm_global_defines.h>

#include <QObject>
#include <QAbstractItemView>

namespace filedialog_core {

class CoreEventsCaller
{
public:
    static void sendViewMode(QWidget *sender, DFMBASE_NAMESPACE::Global::ViewMode mode);
    static void sendSelectFiles(quint64 windowId, const QList<QUrl> &files);
    static void setSidebarItemVisible(const QUrl &url, bool visible);
    static void setSelectionMode(QWidget *sender, const QAbstractItemView::SelectionMode mode);
    static void setEnabledSelectionModes(QWidget *sender, const QList<QAbstractItemView::SelectionMode> &modes);
    static void setMenuDisbaled();
    static QList<QUrl> sendGetSelectedFiles(const quint64 windowID);
};

}

#endif   // COREEVENTSCALLER_H
