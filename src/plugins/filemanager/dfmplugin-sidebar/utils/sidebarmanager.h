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
#ifndef SIDEBARMANAGER_H
#define SIDEBARMANAGER_H

#include "dfmplugin_sidebar_global.h"

#include "services/filemanager/sidebar/sidebar_defines.h"

#include <QObject>
#include <QString>
#include <QMap>

DPSIDEBAR_BEGIN_NAMESPACE

class SideBarManager final : public QObject
{
    Q_OBJECT

public:
    using CdCallback = DSB_FM_NAMESPACE::SideBar::CdActionCallback;
    using ContextMenuCallback = DSB_FM_NAMESPACE::SideBar::ContextMenuCallback;
    using RenameCallback = DSB_FM_NAMESPACE::SideBar::RenameCallback;

public:
    static SideBarManager *instance();

    void runCd(const QString &identifier, quint64 windowId, const QUrl &url);
    void runContextMenu(const QString &identifier, quint64 windowId, const QUrl &url, const QPoint &globalPos);
    void runRename(const QString &identifier, quint64 windowId, const QUrl &url, const QString &name);

    void registerCallback(const QString &identifier,
                          const CdCallback &cd, const ContextMenuCallback &contexMenu, const RenameCallback &rename);

private:
    explicit SideBarManager(QObject *parent = nullptr);
    ~SideBarManager();

private:
    QMap<QString, CdCallback> cdCallbackMap;
    QMap<QString, ContextMenuCallback> menuCallbackMap;
    QMap<QString, RenameCallback> renameCallbackMap;
};

DPSIDEBAR_END_NAMESPACE

#endif   // SIDEBARMANAGER_H
