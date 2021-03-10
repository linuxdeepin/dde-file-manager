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

#include "../dbusservice/dbusadaptor/vault_adaptor.h"
#include <QtCore/QMetaObject>
#include <QtCore/QByteArray>
#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVariant>

/*
 * Implementation of adaptor class VaultAdaptor
 */

VaultAdaptor::VaultAdaptor(VaultManager *parent)
    : QDBusAbstractAdaptor(parent)
{
    // constructor
    setAutoRelaySignals(true);
}

VaultAdaptor::~VaultAdaptor()
{
    // destructor
}

bool VaultAdaptor::checkAuthentication(const QString &type)
{
    // handle method call com.deepin.filemanager.daemon.VaultManager.checkAuthentication
    return parent()->checkAuthentication(type);
}

void VaultAdaptor::clearLockEvent()
{
    // handle method call com.deepin.filemanager.daemon.VaultManager.clearLockEvent
    parent()->clearLockEvent();
}

qulonglong VaultAdaptor::getLastestTime()
{
    // handle method call com.deepin.filemanager.daemon.VaultManager.getLastestTime
    return parent()->getLastestTime();
}

qulonglong VaultAdaptor::getSelfTime()
{
    // handle method call com.deepin.filemanager.daemon.VaultManager.getSelfTime
    return parent()->getSelfTime();
}

bool VaultAdaptor::isLockEventTriggered()
{
    // handle method call com.deepin.filemanager.daemon.VaultManager.isLockEventTriggered
    return parent()->isLockEventTriggered();
}

void VaultAdaptor::setRefreshTime(qulonglong time)
{
    // handle method call com.deepin.filemanager.daemon.VaultManager.setRefreshTime
    parent()->setRefreshTime(time);
}

void VaultAdaptor::sysUserChanged(const QString &curUser)
{
    // handle method call com.deepin.filemanager.daemon.VaultManager.sysUserChanged
    parent()->sysUserChanged(curUser);
}

void VaultAdaptor::triggerLockEvent()
{
    // handle method call com.deepin.filemanager.daemon.VaultManager.triggerLockEvent
    parent()->triggerLockEvent();
}

