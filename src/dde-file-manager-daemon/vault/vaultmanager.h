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


#ifndef VAULTMANAGER_H
#define VAULTMANAGER_H

#include <QDBusContext>
#include <QObject>
#include <QTimer>

class VaultAdaptor;
class VaultClock;

/**
 * @brief The VaultManager class 保险箱管理类
 */
class VaultManager : public QObject, public QDBusContext
{
    Q_OBJECT
public:
    explicit VaultManager(QObject *parent = nullptr);
    ~VaultManager();

    static QString ObjectPath;
    static QString PolicyKitCreateActionId;
    static QString PolicyKitRemoveActionId;

public slots:
    /**
     * @brief slotUserChanged 用户切换槽函数
     * @param curUser
     */
    void sysUserChanged(const QString& curUser);

    /**
     * @brief setRefreshTime 设置保险箱刷新时间
     * @param time
     */
    void setRefreshTime(quint64 time);

    /**
     * @brief getLastestTime 获取保险柜计时
     * @return
     */
    quint64 getLastestTime() const;

    /**
     * @brief getSelfTime 获取自定义时间
     * @return
     */
    quint64 getSelfTime() const;

    /**
    * @brief checkAuthentication 权限验证
    * @param type
    * @return
    */
    bool checkAuthentication(QString type);

    /**
     * @brief isLockEventTriggered 是否存在已触发的锁定事件
     * @return
     */
    bool isLockEventTriggered() const;

    /**
     * @brief triggerLockEvent 触发锁定事件
     */
    void triggerLockEvent();

    /**
     * @brief clearLockEvent 清除锁定事件
     */
    void clearLockEvent();

    /**
     * @brief computerSleep 通过dbus接口获取电脑休眠状态
     * 该函数将主机休眠时间记录到时钟
     * @param bSleep true为正要进入休眠，false为进入唤醒状态
     */
    void computerSleep(bool bSleep);

private:
    /**
     * @brief getCurrentUser 获取当前用户
     * @return
     */
    QString getCurrentUser() const;

    VaultAdaptor* m_vaultAdaptor = nullptr;

    QMap<QString, VaultClock*> m_mapUserClock; // map user and timer.
    VaultClock *m_curVaultClock; // current user clock.
    QString m_curUser; // current system user.
    qint64 m_pcTime; // 主机时间
};

#endif // VAULTMANAGER_H
