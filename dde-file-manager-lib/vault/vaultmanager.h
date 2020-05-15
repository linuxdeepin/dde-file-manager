/*
 * Copyright (C) 2016 ~ 2018 Deepin Technology Co., Ltd.
 *               2016 ~ 2018 dragondjf
 *
 * Author:     dragondjf<dingjiangfeng@deepin.com>
 *
 * Maintainer: dragondjf<dingjiangfeng@deepin.com>
 *             zccrs<zhangjide@deepin.com>
 *             Tangtong<tangtong@deepin.com>
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

#ifndef VAULTMANAGER_H
#define VAULTMANAGER_H

#include "dabstractfilecontroller.h"

#include "dfmglobal.h"

#include <QTimer>

class VaultInterface;
class VaultManager : public QObject
{
    Q_OBJECT

public:

    enum AutoLockState {
        Never = 0,
        FiveMinutes = 5,
        TenMinutes = 10,
        TweentyMinutes = 20
    };

    static VaultManager &getInstance();

    /**
     * @brief autoLockState    自动上锁状态
     * @return                 返回状态值
     */
    AutoLockState autoLockState() const;

    /**
     * @brief autoLock    设置自动锁状态
     * @param lockState   状态值
     * @return
     */
    bool autoLock(AutoLockState lockState);

protected:
    /**
     * @brief setRefreshTime 刷新保险柜最新计时
     * @param time 时间
     * @return
     */
    void dbusSetRefreshTime(quint64 time);

    /**
     * @brief getLastestTime 获取最新计时
     * @return
     */
    quint64 dbusGetLastestTime() const;

    /**
     * @brief isValid 保险箱服务是否开启
     * @return
     */
    bool isValid() const;

protected slots:
    /**
     * @brief refreshAccessTime 刷新保险箱访问时间
     */
    void refreshAccessTime();

    /**
     * @brief processAutoLock 处理自动加锁
     */
    void processAutoLock();

private:
    explicit VaultManager(QObject *parent = nullptr);
    ~VaultManager();

private:
    VaultInterface* m_vaultInterface = nullptr; // 交互接口
    qulonglong m_lastestTime; // 最新计时
    AutoLockState m_lockState; // 自动锁状态

    DAbstractFileInfoPointer m_rootFileInfo; // 根目录文件信息

    QTimer m_refreshTimer; // 保险箱访问时间刷新
    QTimer m_alarmClock; // 自动锁计时器
};

#endif // VAULTMANAGER_H
