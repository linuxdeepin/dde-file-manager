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
#ifndef SIDEBARUNICASTRECEIVER_H
#define SIDEBARUNICASTRECEIVER_H

#include "dfmplugin_sidebar_global.h"
#include "services/filemanager/sidebar/sidebar_defines.h"

#include <QObject>

DPSIDEBAR_BEGIN_NAMESPACE

class SideBarUnicastReceiver : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(SideBarUnicastReceiver)

public:
    static SideBarUnicastReceiver *instance();
    void connectService();

public slots:
    void invokeAddItem(const DSB_FM_NAMESPACE::SideBar::ItemInfo &info);
    void invokeRemoveItem(const QUrl &url);
    void invokeUpdateItem(const QUrl &url, const DSB_FM_NAMESPACE::SideBar::ItemInfo &info);
    void invokeUpdateItemName(const QUrl &url, const QString &newName, bool editable);
    void invokeUpdateItemIcon(const QUrl &url, const QIcon &newIcon);
    void invokeInsertItem(int index,
                          const DSB_FM_NAMESPACE::SideBar::ItemInfo &info);
    void invokeTriggerItemEdit(quint64 winId, const QUrl &url);
    bool invokeRegisterSortFunc(const QString &subGroup, DSB_FM_NAMESPACE::SideBar::SortFunc func);

private:
    explicit SideBarUnicastReceiver(QObject *parent = nullptr);
};

DPSIDEBAR_END_NAMESPACE

#endif   // SIDEBARUNICASTRECEIVER_H
