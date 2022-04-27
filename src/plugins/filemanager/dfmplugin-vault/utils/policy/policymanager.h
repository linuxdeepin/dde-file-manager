/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     lixiang<lixianga@uniontech.com>
 *
 * Maintainer: lixiang<lixianga@uniontech.com>
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
#ifndef POLICYMANAGER_H
#define POLICYMANAGER_H

#include "dfmplugin_vault_global.h"

#include <QObject>

DPVAULT_BEGIN_NAMESPACE
class PolicyManager : public QObject
{
    Q_OBJECT
public:
    //! 保险箱当前页面标记
    enum VaultPageMark {
        kUnknown,
        kCreateVaultPage,
        kCreateVaultPage1,
        kUnlockVaultPage,
        kRetrievePassWordPage,
        kDeleteFilePage,
        kDeleteVaultPage,
        kCopyFilePage,
        kClipboardPage,
        kVaultPage
    };

private:
    explicit PolicyManager(QObject *parent = nullptr);

public:
    /*!
     * \brief getVaultPolicy 获取当前策略
     * \return 返回保险箱是否隐藏  1隐藏 2显示
     */
    static int getVaultPolicy();

    /*!
     * \brief setVaultPolicyState 设置策略是否可用
     * \param policyState 1策略可用 2策略不可用
     * \return 设置是否成功
     */
    static bool setVaultPolicyState(int policyState);

    /*!
     * \brief  获取当前所处保险箱页面
     * \return 返回当前页面标识
     */
    static VaultPageMark getVaultCurrentPageMark();

    /*!
     * \brief 设置当前所处保险箱页面
     * \param mark 页面标识
     */
    static void setVauleCurrentPageMark(VaultPageMark mark);

    /*!
     * \brief isVaultVisiable 获取保险箱显示状态
     * \return true显示、false隐藏
     */
    static bool isVaultVisiable();

    static PolicyManager *instance();

public slots:
    //! 保险箱策略处理函数
    void slotVaultPolicy();

private:
    //! 用于记录当前保险箱所处页面标识
    static VaultPageMark recordVaultPageMark;

    static bool vaultVisiable;
};
DPVAULT_END_NAMESPACE
#endif   // POLICYMANAGER_H
