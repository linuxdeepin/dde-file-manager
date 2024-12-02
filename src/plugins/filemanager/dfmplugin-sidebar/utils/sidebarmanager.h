// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SIDEBARMANAGER_H
#define SIDEBARMANAGER_H

#include "dfmplugin_sidebar_global.h"

#include <QObject>
#include <QString>
#include <QMap>

DPSIDEBAR_BEGIN_NAMESPACE
class SideBarItem;

class SideBarManager final : public QObject
{
    Q_OBJECT

public:
    static SideBarManager *instance();

    void runCd(SideBarItem *item, quint64 windowId);
    void runContextMenu(SideBarItem *item, quint64 windowId, const QPoint &globalPos);
    void runRename(SideBarItem *item, quint64 windowId, const QString &name);
    void openFolderInASeparateProcess(const QUrl &url);

private:
    explicit SideBarManager(QObject *parent = nullptr);
    ~SideBarManager();

private:
};

DPSIDEBAR_END_NAMESPACE

#endif   // SIDEBARMANAGER_H
