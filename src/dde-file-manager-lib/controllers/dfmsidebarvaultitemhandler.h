// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "dfmsidebariteminterface.h"

#define SIDEBAR_ID_VAULT "__vault"

class DFileManagerWindow;
class DFileMenu;
class DFMVaultUnlockPages;

DFM_BEGIN_NAMESPACE
/**
 * @brief 实现侧边栏保险箱
 */
class DFMSideBarVaultItemHandler : public DFMSideBarItemInterface
{
public:
    /**
     * @brief 创建该类实例
     * @param pathKey
     * @return 创建的实例指针
     */
    static DFMSideBarItem *createItem(const QString &pathKey);

    /**
     * @brief 构造函数
     * @param parent
     */
    explicit DFMSideBarVaultItemHandler(QObject *parent = nullptr);

    /**
     * @brief 打开操作
     * @param sidebar
     * @param item
     */
    void cdAction(const DFMSideBar *sidebar, const DFMSideBarItem *item) override;

    /**
     * @brief 返回右键菜单
     * @param sidebar
     * @param item
     * @return 右键菜单
     */
    QMenu *contextMenu(const DFMSideBar *sidebar, const DFMSideBarItem *item) override;

    /**
     * @brief 产生菜单
     * @param topWidget
     * @param sender
     * @return 菜单项目
     */
    DFileMenu *generateMenu(QWidget *topWidget, const DFMSideBar *sender = nullptr);

private:
    /**
     * @brief 立即上锁
     * @return 上锁是否成功
     */
    bool lockNow(DFileManagerWindow *wnd);

    /**
     * @brief 自动上锁
     * @param lockState 加锁状态
     * @return 自动上锁是否成功
     */
    bool autoLock(int lockState);

private:
    void showView(QWidget *wndPtr, QString host);

    DFMVaultUnlockPages *m_vaultUnlockPage { nullptr };
};

DFM_END_NAMESPACE
