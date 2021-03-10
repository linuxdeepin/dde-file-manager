/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     luzhen<luzhen@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             luzhen<luzhen@uniontech.com>
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
