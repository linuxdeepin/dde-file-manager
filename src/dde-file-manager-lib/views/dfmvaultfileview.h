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

#pragma once

#include "dfileview.h"
#include "interfaces/dfilemenu.h"

/**
 * @brief The DFMVaultFileView class
 * The class is vault main view, list files in vault directory.
 * It will open pages or dialog according to current encrypted
 * state when setRootUrl invoked.
 */
class DFMVaultFileView : public DFileView
{
    Q_OBJECT
public:
    explicit DFMVaultFileView(QWidget *parent = nullptr);

    /**
     * @brief setRootUrl 设置根目录地址
     * @param url        根目录地址
     * @return           是否设置成功，如果是page相关的地址，返回false
     */
    bool setRootUrl(const DUrl &url) override;

public slots:
    /**
     * @brief onLeaveVault 离开保险箱，当上锁/删除保险箱时调用
     */
    void onLeaveVault(int state);

    /**
    * @brief IgnoreDragEvent 忽略拖拽事件
    */
    void IgnoreDragEvent();

protected:
    /**
     * @brief eventFilter 事件过滤器，用于获取保险箱操作事件
     * @param obj
     * @param event       事件指针
     * @return
     */
    bool eventFilter(QObject *obj, QEvent *event) override;

protected slots:
    // 保险箱单独实现父目录删除后，目录跳转方法
    void onRootUrlDeleted(const DUrl &rootUrl) override;
};
