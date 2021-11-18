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

#include "vaultbruteforceprevention.h"
#include "dbusadaptor/vaultbruteforceprevention_adaptor.h"

QString VaultBruteForcePrevention::ObjectPath = "/com/deepin/filemanager/daemon/VaultManager2";

VaultBruteForcePrevention::VaultBruteForcePrevention(QObject *parent) : QObject(parent)
{
    QDBusConnection::systemBus().registerObject(ObjectPath, this);
    m_vaultAdaptor = new VaultBruteForcePreventionAdaptor(this);
}

int VaultBruteForcePrevention::getLeftoverErrorInputTimes(int userID)
{
    if (!isValidInvoker())
        return -1;
    if (!m_mapLeftoverInputTimes.contains(userID))
        m_mapLeftoverInputTimes[userID] = ERROR_INPUT_TIMES;
    return m_mapLeftoverInputTimes[userID];
}

void VaultBruteForcePrevention::leftoverErrorInputTimesMinusOne(int userID)
{
    if (!isValidInvoker())
        return;
    if (!m_mapLeftoverInputTimes.contains(userID))
        m_mapLeftoverInputTimes[userID] = ERROR_INPUT_TIMES;
    --m_mapLeftoverInputTimes[userID];
}

void VaultBruteForcePrevention::restoreLeftoverErrorInputTimes(int userID)
{
    if (!isValidInvoker())
        return;
    m_mapLeftoverInputTimes[userID] = ERROR_INPUT_TIMES;
}

void VaultBruteForcePrevention::startTimerOfRestorePasswordInput(int userID)
{
    if (!isValidInvoker())
        return;
    m_mapTimer.insert(startTimer(TIMER_OUT_TIME), userID);
}

int VaultBruteForcePrevention::getNeedWaitMinutes(int userID)
{
    if (!isValidInvoker())
        return 100;
    if (!m_mapNeedMinutes.contains(userID))
        m_mapNeedMinutes[userID] = TOTAL_WAIT_TIME;
    return m_mapNeedMinutes[userID];
}

void VaultBruteForcePrevention::restoreNeedWaitMinutes(int userID)
{
    if (!isValidInvoker())
        return;
    m_mapNeedMinutes[userID] = TOTAL_WAIT_TIME;
}

void VaultBruteForcePrevention::timerEvent(QTimerEvent *event)
{
    int timerID = event->timerId();
    if (m_mapTimer.contains(timerID)) {
        int userID = m_mapTimer[timerID];
        if (!m_mapNeedMinutes.contains(userID))
            m_mapNeedMinutes[userID] = TOTAL_WAIT_TIME;
        --m_mapNeedMinutes[userID];
        if (m_mapNeedMinutes[userID] < 1) {
            killTimer(timerID);
            m_mapTimer.remove(timerID);
            // 密码剩余输入次数还原，需要等待的分钟数还原
            restoreLeftoverErrorInputTimes(userID);
            restoreNeedWaitMinutes(userID);
        }
    }
}

bool VaultBruteForcePrevention::isValidInvoker()
{
#if 0
    // 暂时屏蔽，后期优化
    // 1. 该白名单方案会造成调试运行保险箱，保险箱进程不再白名单内，导致无法开锁
    // 2. 该写法存在逻辑问题，导致了bug#103685
    static QStringList VaultwhiteProcess = {"/usr/bin/dde-file-manager", "/usr/bin/dde-desktop", "/usr/bin/dde-select-dialog-wayland", "/usr/bin/dde-select-dialog-x11"};
    uint pid = connection().interface()->servicePid(message().service()).value();
    QFileInfo f(QString("/proc/%1/exe").arg(pid));
    if (!f.exists())
        return false;
    return VaultwhiteProcess.contains(f.canonicalFilePath());
#endif
    return true;
}
