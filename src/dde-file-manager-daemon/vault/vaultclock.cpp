// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "vault/vaultclock.h"
#include "dbusadaptor/vault_adaptor.h"
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
