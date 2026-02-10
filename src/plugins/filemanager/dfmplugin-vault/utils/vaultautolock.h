// SPDX-FileCopyrightText: 2023 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef VAULTAUTOLOCK_H
#define VAULTAUTOLOCK_H
#include "dfmplugin_vault_global.h"

#include <QObject>
#include <QTimer>

namespace dfmplugin_vault {
class VaultAutoLock : public QObject
{
    Q_OBJECT
public:
    enum AutoLockState {
        kNever = 0,
        kFiveMinutes = 5,
        kTenMinutes = 10,
        kTwentyMinutes = 20
    };
    explicit VaultAutoLock(QObject *parent = nullptr);

public:
    //! 自动上锁与暴力输入
    AutoLockState getAutoLockState() const;

    bool autoLock(AutoLockState lockState);

    bool isValid() const;

    void refreshAccessTime();

    void resetConfig();

public slots:
    //! 自动上锁与暴力输入
    void processAutoLock();

    void slotUnlockVault(int state);

    void slotLockVault(int state);

    void processLockEvent();

    void slotLockEvent(const QString &user);

protected:
    /**
     * @brief loadConfig 加载配置文件
     */
    void loadConfig();

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
     * @brief dbusGetSelfTime 获取自定义时间
     * @return
     */
    quint64 dbusGetSelfTime() const;

public:
    static VaultAutoLock *instance();

private:
    //! auto lock clock
    QTimer alarmClock;

    //! auto lock state
    AutoLockState autoLockState;

    bool isCacheTimeReloaded;
};
}
#endif   // VAULTAUTOLOCK_H
