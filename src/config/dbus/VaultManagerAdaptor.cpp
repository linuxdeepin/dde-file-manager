// SPDX-FileCopyrightText: 2024 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "src/config/dbus/VaultManagerAdaptor.h"
#include <QtCore/QMetaObject>
#include <QtCore/QByteArray>
#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVariant>

/*
 * Implementation of adaptor class VaultManagerAdaptor
 */

VaultManagerAdaptor::VaultManagerAdaptor(QObject *parent)
    : QDBusAbstractAdaptor(parent)
{
    // constructor
    setAutoRelaySignals(true);
}

VaultManagerAdaptor::~VaultManagerAdaptor()
{
    // destructor
}

void VaultManagerAdaptor::ClearLockEvent()
{
    // handle method call org.deepin.Filemanager.Daemon.VaultManager.ClearLockEvent
    QMetaObject::invokeMethod(parent(), "ClearLockEvent");
}

void VaultManagerAdaptor::ComputerSleep(bool bSleep)
{
    // handle method call org.deepin.Filemanager.Daemon.VaultManager.ComputerSleep
    QMetaObject::invokeMethod(parent(), "ComputerSleep", Q_ARG(bool, bSleep));
}

qulonglong VaultManagerAdaptor::GetLastestTime()
{
    // handle method call org.deepin.Filemanager.Daemon.VaultManager.GetLastestTime
    qulonglong out0;
    QMetaObject::invokeMethod(parent(), "GetLastestTime", Q_RETURN_ARG(qulonglong, out0));
    return out0;
}

int VaultManagerAdaptor::GetLeftoverErrorInputTimes(int userID)
{
    // handle method call org.deepin.Filemanager.Daemon.VaultManager.GetLeftoverErrorInputTimes
    int out0;
    QMetaObject::invokeMethod(parent(), "GetLeftoverErrorInputTimes", Q_RETURN_ARG(int, out0), Q_ARG(int, userID));
    return out0;
}

int VaultManagerAdaptor::GetNeedWaitMinutes(int userID)
{
    // handle method call org.deepin.Filemanager.Daemon.VaultManager.GetNeedWaitMinutes
    int out0;
    QMetaObject::invokeMethod(parent(), "GetNeedWaitMinutes", Q_RETURN_ARG(int, out0), Q_ARG(int, userID));
    return out0;
}

qulonglong VaultManagerAdaptor::GetSelfTime()
{
    // handle method call org.deepin.Filemanager.Daemon.VaultManager.GetSelfTime
    qulonglong out0;
    QMetaObject::invokeMethod(parent(), "GetSelfTime", Q_RETURN_ARG(qulonglong, out0));
    return out0;
}

bool VaultManagerAdaptor::IsLockEventTriggered()
{
    // handle method call org.deepin.Filemanager.Daemon.VaultManager.IsLockEventTriggered
    bool out0;
    QMetaObject::invokeMethod(parent(), "IsLockEventTriggered", Q_RETURN_ARG(bool, out0));
    return out0;
}

void VaultManagerAdaptor::LeftoverErrorInputTimesMinusOne(int userID)
{
    // handle method call org.deepin.Filemanager.Daemon.VaultManager.LeftoverErrorInputTimesMinusOne
    QMetaObject::invokeMethod(parent(), "LeftoverErrorInputTimesMinusOne", Q_ARG(int, userID));
}

void VaultManagerAdaptor::RestoreLeftoverErrorInputTimes(int userID)
{
    // handle method call org.deepin.Filemanager.Daemon.VaultManager.RestoreLeftoverErrorInputTimes
    QMetaObject::invokeMethod(parent(), "RestoreLeftoverErrorInputTimes", Q_ARG(int, userID));
}

void VaultManagerAdaptor::RestoreNeedWaitMinutes(int userID)
{
    // handle method call org.deepin.Filemanager.Daemon.VaultManager.RestoreNeedWaitMinutes
    QMetaObject::invokeMethod(parent(), "RestoreNeedWaitMinutes", Q_ARG(int, userID));
}

void VaultManagerAdaptor::SetRefreshTime(qulonglong time)
{
    // handle method call org.deepin.Filemanager.Daemon.VaultManager.SetRefreshTime
    QMetaObject::invokeMethod(parent(), "SetRefreshTime", Q_ARG(qulonglong, time));
}

void VaultManagerAdaptor::StartTimerOfRestorePasswordInput(int userID)
{
    // handle method call org.deepin.Filemanager.Daemon.VaultManager.StartTimerOfRestorePasswordInput
    QMetaObject::invokeMethod(parent(), "StartTimerOfRestorePasswordInput", Q_ARG(int, userID));
}

void VaultManagerAdaptor::SysUserChanged(const QString &curUser)
{
    // handle method call org.deepin.Filemanager.Daemon.VaultManager.SysUserChanged
    QMetaObject::invokeMethod(parent(), "SysUserChanged", Q_ARG(QString, curUser));
}

void VaultManagerAdaptor::TriggerLockEvent()
{
    // handle method call org.deepin.Filemanager.Daemon.VaultManager.TriggerLockEvent
    QMetaObject::invokeMethod(parent(), "TriggerLockEvent");
}

