// SPDX-FileCopyrightText: 2023 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef VAULTMANAGERDBUS_H
#define VAULTMANAGERDBUS_H

#include "private/vaultclock.h"

#include <QObject>
#include <QMap>
#include <QDBusMessage>
#include <QDBusContext>

class VaultManagerDBus : public QObject, public QDBusContext
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.deepin.Filemanager.Daemon.VaultManager")
public:
    explicit VaultManagerDBus(QObject *parent = nullptr);

public Q_SLOTS:
    /*!
     * \brief  用户切换槽函数
     * \param curUser
     */
    void SysUserChanged(const QString &curUser);

    /*!
     * \brief  设置保险箱刷新时间
     * \param time
     */
    void SetRefreshTime(quint64 time);

    /*!
     * \brief  获取保险柜计时
     * \return
     */
    quint64 GetLastestTime() const;

    /*!
     * \brief  获取自定义时间
     * \return
     */
    quint64 GetSelfTime() const;

    /*!
     * \brief  是否存在已触发的锁定事件
     * \return
     */
    bool IsLockEventTriggered() const;

    /*!
     * \brief  触发锁定事件
     */
    void TriggerLockEvent();

    /*!
     * \brief  清除锁定事件
     */
    void ClearLockEvent();

    /*!
     * \brief  通过dbus接口获取电脑休眠状态
     * 该函数将主机休眠时间记录到时钟
     * \param bSleep true为正要进入休眠，false为进入唤醒状态
     */
    void ComputerSleep(bool bSleep);

    /*!
     * \brief 获得保险箱剩余错误密码输入次数
     * \return
     */
    int GetLeftoverErrorInputTimes(int userID);

    /*!
     * \brief  保险箱剩余错误密码输入次数减1
     */
    void LeftoverErrorInputTimesMinusOne(int userID);

    /*!
     * \brief  保险箱剩余错误密码输入次数还原
     */
    void RestoreLeftoverErrorInputTimes(int userID);

    /*!
     * \brief  开启恢复密码输入定时器
     */
    void StartTimerOfRestorePasswordInput(int userID);

    /*!
     * \brief  获得保险箱再次输入密码需要等待的分钟数
     * \return
     */
    int GetNeedWaitMinutes(int userID);

    /*!
     * \brief  保险箱再次输入密码的等待分钟数还原
     */
    void RestoreNeedWaitMinutes(int userID);

Q_SIGNALS:
    /**
     * @brief LockEventTriggered 锁屏信号
     * @param user
     */
    void LockEventTriggered(QString user);
    void ChangedVaultState(const QVariantMap &map);

protected:
    void timerEvent(QTimerEvent *event) override;

private:
    /*!
     * \brief  判断调用者是否是白名单进程
     * \return
     */
    bool IsValidInvoker();

    /*!
     * \brief  获取当前用户
     * \return
     */
    QString GetCurrentUser() const;

    /*!
     * \brief  内部还原保险箱剩余错误密码输入次数（不检查权限）
     * \param userID
     */
    void restoreLeftoverErrorInputTimes(int userID);

    /*!
     * \brief  内部还原保险箱需要等待的分钟数（不检查权限）
     * \param userID
     */
    void restoreNeedWaitMinutes(int userID);

    QMap<QString, VaultClock *> mapUserClock {};   // map user and timer.
    VaultClock *curVaultClock { nullptr };   // current user clock.
    QString currentUser {};   // current system user.
    qint64 pcTime { 0 };   // 主机时间

    // 记录保险箱剩余的错误密码输入次数
    QMap<int, int> mapLeftoverInputTimes {};

    // 剩余时间定时器，key:定时器ID value：用户ID
    QMap<int, int> mapTimer {};

    // 记录恢复密码输入还需要的分钟数
    QMap<int, int> mapNeedMinutes {};
};
#endif   // VAULTMANAGERDBUS_H
