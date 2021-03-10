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

#include "vault/vaultclock.h"
#include "dbusservice/dbusadaptor/vault_adaptor.h"
#include <QDBusConnection>
#include <QDBusPendingCall>
#include <QDBusVariant>
#include <QProcess>
#include <QDebug>

VaultClock::VaultClock(QObject *parent)
    : QObject(parent)
    , m_selfTime(0)
{
    // Create self clock.
    connect(&m_selfTimer, &QTimer::timeout, this, &VaultClock::tick);
    m_selfTimer.setInterval(1000);
    m_selfTimer.start();
}

VaultClock::~VaultClock()
{
    m_selfTimer.stop();
}

void VaultClock::setRefreshTime(quint64 time)
{
    m_lastestTime = time;
}

quint64 VaultClock::getLastestTime() const
{
    return m_lastestTime;
}

quint64 VaultClock::getSelfTime() const
{
    return m_selfTime;
}

void VaultClock::addTickTime(qint64 seconds)
{
    m_selfTime += static_cast<quint64>(seconds);
}

bool VaultClock::isLockEventTriggered() const
{
    return m_isLockEventTriggerd;
}

void VaultClock::triggerLockEvent()
{
    m_isLockEventTriggerd = true;
}

void VaultClock::clearLockEvent()
{
    m_isLockEventTriggerd = false;
}

void VaultClock::tick()
{
    m_selfTime++;
}
