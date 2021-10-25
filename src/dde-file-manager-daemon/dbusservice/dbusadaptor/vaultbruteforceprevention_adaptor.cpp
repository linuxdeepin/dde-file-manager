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

#include "../dbusservice/dbusadaptor/vaultbruteforceprevention_adaptor.h"
#include <QtCore/QMetaObject>
#include <QtCore/QByteArray>
#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVariant>

/*
 * Implementation of adaptor class VaultBruteForcePreventionAdaptor
 */

VaultBruteForcePreventionAdaptor::VaultBruteForcePreventionAdaptor(VaultBruteForcePrevention *parent)
    : QDBusAbstractAdaptor(parent)
{
    // constructor
    setAutoRelaySignals(true);
}

VaultBruteForcePreventionAdaptor::~VaultBruteForcePreventionAdaptor()
{
    // destructor
}

int VaultBruteForcePreventionAdaptor::getLeftoverErrorInputTimes(int userID)
{
    // handle method call com.deepin.filemanager.daemon.VaultManager2.getLeftoverErrorInputTimes
    return parent()->getLeftoverErrorInputTimes(userID);
}

int VaultBruteForcePreventionAdaptor::getNeedWaitMinutes(int userID)
{
    // handle method call com.deepin.filemanager.daemon.VaultManager2.getNeedWaitMinutes
    return parent()->getNeedWaitMinutes(userID);
}

void VaultBruteForcePreventionAdaptor::leftoverErrorInputTimesMinusOne(int userID)
{
    // handle method call com.deepin.filemanager.daemon.VaultManager2.leftoverErrorInputTimesMinusOne
    parent()->leftoverErrorInputTimesMinusOne(userID);
}

void VaultBruteForcePreventionAdaptor::restoreLeftoverErrorInputTimes(int userID)
{
    // handle method call com.deepin.filemanager.daemon.VaultManager2.restoreLeftoverErrorInputTimes
    parent()->restoreLeftoverErrorInputTimes(userID);
}

void VaultBruteForcePreventionAdaptor::restoreNeedWaitMinutes(int userID)
{
    // handle method call com.deepin.filemanager.daemon.VaultManager2.restoreNeedWaitMinutes
    parent()->restoreNeedWaitMinutes(userID);
}

void VaultBruteForcePreventionAdaptor::startTimerOfRestorePasswordInput(int userID)
{
    // handle method call com.deepin.filemanager.daemon.VaultManager2.startTimerOfRestorePasswordInput
    parent()->startTimerOfRestorePasswordInput(userID);
}

