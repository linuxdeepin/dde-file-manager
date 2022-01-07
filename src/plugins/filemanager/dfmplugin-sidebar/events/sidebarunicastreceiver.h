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
    void invokeAddItem(const DSB_FM_NAMESPACE::SideBar::ItemInfo &info,
                       DSB_FM_NAMESPACE::SideBar::CdActionCallback cdFunc,
                       DSB_FM_NAMESPACE::SideBar::ContextMenuCallback menuFunc,
                       DSB_FM_NAMESPACE::SideBar::RenameCallback renameFunc);

private:
    explicit SideBarUnicastReceiver(QObject *parent = nullptr);
};

DPSIDEBAR_END_NAMESPACE

#endif   // SIDEBARUNICASTRECEIVER_H
