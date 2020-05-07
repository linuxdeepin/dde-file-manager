/*
 * Copyright (C) 2020 Deepin Technology Co., Ltd.
 *
 * Author:     Lu Zhen <luzhen@uniontech.com>
 *
 * Maintainer: Lu Zhen <luzhen@uniontech.com>
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

#pragma once

#include "dfmsidebariteminterface.h"

#define SIDEBAR_ID_VAULT "__vault"

DFM_BEGIN_NAMESPACE

// 实现侧边栏保险箱Item
class DFMSideBarVaultItemHandler : public DFMSideBarItemInterface
{
public:
    static DFMSideBarItem * createItem(const QString &pathKey);

    DFMSideBarVaultItemHandler(QObject *parent = nullptr);

    void cdAction(const DFMSideBar *sidebar, const DFMSideBarItem* item) override;
    QMenu *contextMenu(const DFMSideBar *sidebar, const DFMSideBarItem* item) override;

private:
    // 立即上锁
    bool lockNow();

    // 自动上锁
    bool autoLock(uint minutes);

    // 显示删除保险箱页面
    bool showDeleteVaultView();

    // 显示解锁页面
    bool showUnLockView();

    // 显示凭证页面
    bool showCertificateView();
};

DFM_END_NAMESPACE
