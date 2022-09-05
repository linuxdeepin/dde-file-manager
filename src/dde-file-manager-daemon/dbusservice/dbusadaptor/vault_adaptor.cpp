// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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

