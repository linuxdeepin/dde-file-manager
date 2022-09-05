// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef VAULTLOCKMANAGER_H
#define VAULTLOCKMANAGER_H

#include "dabstractfilecontroller.h"

#include "dfmglobal.h"

//#define AUTOLOCK_TEST
class VaultInterface;
class VaultLockManagerPrivate;
class VaultLockManager : public QObject
{
    Q_OBJECT

public:
    enum AutoLockState {
        Never = 0,
        FiveMinutes = 5,
        TenMinutes = 10,
        TwentyMinutes = 20
    };

    static VaultLockManager &getInstance();

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

    /**
     * @brief isValid 保险箱服务是否开启
     * @return
     */
    bool isValid() const;

    /**
     * @brief refreshAccessTime 刷新保险箱访问时间
     */
    void refreshAccessTime();

    /**
    * @brief checkAuthentication 保险箱权限认证
    */
    bool checkAuthentication(QString type);

    /**
     * @brief resetConfig 重置保险箱配置
     */
    void resetConfig();

protected slots:
    /**
     * @brief processAutoLock 处理自动加锁
     */
    void processAutoLock();

    /**
     * @brief slotLockVault 加锁状态槽函数
     */
    void slotLockVault(int msg);

    /**
     * @brief slotUnlockVault 解锁状态槽函数
     */
    void slotUnlockVault(int msg);

    /**
     * @brief processLockEvent 处理加锁事件
     */
    void processLockEvent();

    /**
     * @brief slotLockEvent 上锁事件
     * @param user
     */
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

private:
    explicit VaultLockManager(QObject *parent = nullptr);
    QSharedPointer<VaultLockManagerPrivate> d_ptr;

    Q_DECLARE_PRIVATE(VaultLockManager)
};

#endif // VAULTLOCKMANAGER_H
