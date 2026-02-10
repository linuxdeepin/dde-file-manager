// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef VAULTCLOCK_H
#define VAULTCLOCK_H

#include <QObject>
#include <QTimer>

/**
 * @brief The VaultClock class
 */
class VaultClock : public QObject
{
    Q_OBJECT
public:
    explicit VaultClock(QObject *parent = nullptr);
    ~VaultClock();
    static QString vaultBasePath();

public Q_SLOTS:
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
     * @brief addTickTime 增加时钟
     * @param seconds
     */
    void addTickTime(qint64 seconds);

protected Q_SLOTS:
    /**
     * @brief tick 秒针
     */
    void tick();

private:
    quint64 lastestTime { 0 };   // latest time

    QTimer selfTimer;
    quint64 selfTime { 0 };

    bool isLockEventTriggerd { false };
};

#endif // VAULTCLOCK_H
