/*
* Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
*
* Author:     gongheng <gongheng@uniontech.com>
*
* Maintainer: zhengyouge <zhengyouge@uniontech.com>
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
