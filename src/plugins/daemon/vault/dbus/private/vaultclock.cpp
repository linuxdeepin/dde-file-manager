// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "vaultclock.h"

#include <QDir>

VaultClock::VaultClock(QObject *parent)
    : QObject(parent)
{
    connect(&selfTimer, &QTimer::timeout, this, &VaultClock::tick);
    selfTimer.setInterval(1000);
    selfTimer.start();
}

VaultClock::~VaultClock()
{
    selfTimer.stop();
}

void VaultClock::setRefreshTime(quint64 time)
{
    lastestTime = time;
}

quint64 VaultClock::getLastestTime() const
{
    return lastestTime;
}

quint64 VaultClock::getSelfTime() const
{
    return selfTime;
}

bool VaultClock::isLockEventTriggered() const
{
    return isLockEventTriggerd;
}

void VaultClock::triggerLockEvent()
{
    isLockEventTriggerd = true;
}

void VaultClock::clearLockEvent()
{
    isLockEventTriggerd = false;
}

void VaultClock::addTickTime(qint64 seconds)
{
    selfTime += static_cast<quint64>(seconds);
}

QString VaultClock::vaultBasePath()
{
    static QString path = QString(QDir::homePath() + QString("/.config/Vault"));   //!! 获取保险箱创建的目录地址
    return path;
}

void VaultClock::tick()
{
    selfTime++;
}
