// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef VAULTHELPER_H
#define VAULTHELPER_H

#include "dfmglobal.h"
#include <QObject>

DFM_BEGIN_NAMESPACE
class VaultHelper : public QObject
{
    Q_OBJECT
public:
    explicit VaultHelper(QObject *parent = nullptr);

    /**
     * @brief topVaultTasks Is there a vault task, top it if exist.
     * @return bool
     */
    static bool topVaultTasks();

    /**
     * @brief killVaultTasks Kill all vault tasks.
     * @return bool
     */
    static bool killVaultTasks();

    /**
     * @brief isVaultEnabled
     * @return
     */
    static bool isVaultEnabled();

    // 定义静态变量，记录当前保险箱是否处于模态弹窗状态
    static bool isModel;

signals:

public slots:
};

DFM_END_NAMESPACE

#endif // VAULTHELPER_H
