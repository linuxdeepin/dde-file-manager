/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
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
#ifndef COREEVENTSCALLER_H
#define COREEVENTSCALLER_H

#include "filedialogplugin_core_global.h"

#include "dfm-base/dfm_global_defines.h"

#include <QObject>
#include <QAbstractItemView>

DIALOGCORE_BEGIN_NAMESPACE

class CoreEventsCaller
{
public:
    static void sendViewMode(QWidget *sender, DFMBASE_NAMESPACE::Global::ViewMode mode);
    static void sendSelectFiles(quint64 windowId, const QList<QUrl> &files);
    static void setSidebarItemVisible(const QUrl &url, bool visible);
    static void setSelectionMode(QWidget *sender, const QAbstractItemView::SelectionMode mode);
    static void setEnabledSelectionModes(QWidget *sender, const QList<QAbstractItemView::SelectionMode> &modes);
};

DIALOGCORE_END_NAMESPACE

#endif   // COREEVENTSCALLER_H
