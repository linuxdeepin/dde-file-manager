// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef POLICYMANAGER_H
#define POLICYMANAGER_H

#include "dfmplugin_vault_global.h"

#include <QObject>

namespace dfmplugin_vault {
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
    static VaultPolicyState getVaultPolicy();

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
}
#endif   // POLICYMANAGER_H
