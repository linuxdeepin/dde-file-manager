/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     gongheng<gongheng@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             gongheng<gongheng@uniontech.com>
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

#ifndef VAULTBRUTEFORCEPREVENTION_H
#define VAULTBRUTEFORCEPREVENTION_H

#define ERROR_INPUT_TIMES 6             // 错误次数
#define TIMER_OUT_TIME 60*1000          // 计时器超时时间/ms
#define TOTAL_WAIT_TIME 10              // 需要等待的分钟数

#include <QObject>
#include <QTimerEvent>
#include <QMap>
#include <QDBusContext>

class VaultBruteForcePreventionAdaptor;

class VaultBruteForcePrevention : public QObject, public QDBusContext
{
    Q_OBJECT
public:
    explicit VaultBruteForcePrevention(QObject *parent = nullptr);

    static QString ObjectPath;

public slots:
    /**
     * @brief getLeftoverErrorInputTimes 获得保险箱剩余错误密码输入次数
     * @return
     */
    int getLeftoverErrorInputTimes(int userID);

    /**
     * @brief leftoverErrorInputTimesMinusOne 保险箱剩余错误密码输入次数减1
     */
    void leftoverErrorInputTimesMinusOne(int userID);

    /**
     * @brief restoreLeftoverErrorInputTimes 保险箱剩余错误密码输入次数还原
     */
    void restoreLeftoverErrorInputTimes(int userID);

    /**
     * @brief startTimerOfRestorePasswordInput 开启恢复密码输入定时器
     */
    void startTimerOfRestorePasswordInput(int userID);

    /**
     * @brief getNeedWaitMinutes 获得保险箱再次输入密码需要等待的分钟数
     * @return
     */
    int getNeedWaitMinutes(int userID);

    /**
     * @brief restoreNeedWaitMinutes 保险箱再次输入密码的等待分钟数还原
     */
    void restoreNeedWaitMinutes(int userID);

protected:
    void timerEvent(QTimerEvent *event) override;

private:
    /**
     * @brief isValidInvoker 判断调用者是否是白名单进程
     * @return
     */
    bool isValidInvoker();

private:
    // 记录保险箱剩余的错误密码输入次数
    QMap<int, int> m_mapLeftoverInputTimes {};

    // 剩余时间定时器，key:定时器ID value：用户ID
    QMap<int, int> m_mapTimer {};

    // 记录恢复密码输入还需要的分钟数
    QMap<int, int> m_mapNeedMinutes {};

    VaultBruteForcePreventionAdaptor *m_vaultAdaptor {nullptr};
};

#endif // VAULTBRUTEFORCEPREVENTION_H
