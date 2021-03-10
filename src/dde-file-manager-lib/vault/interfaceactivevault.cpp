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

#include "interfaceactivevault.h"
#include "operatorcenter.h"

InterfaceActiveVault::InterfaceActiveVault(QObject *parent)
    : QObject(parent)
{

}

InterfaceActiveVault::~InterfaceActiveVault()
{

}

QString InterfaceActiveVault::getPassword()
{
    return OperatorCenter::getInstance()->getSaltAndPasswordCipher();
}

bool InterfaceActiveVault::getPasswordHint(QString &passwordHint)
{
    return OperatorCenter::getInstance()->getPasswordHint(passwordHint);
}

bool InterfaceActiveVault::checkPassword(const QString &password, QString &cipher)
{
    return OperatorCenter::getInstance()->checkPassword(password, cipher);
}

bool InterfaceActiveVault::checkUserKey(const QString &userKey, QString &cipher)
{
    return OperatorCenter::getInstance()->checkUserKey(userKey, cipher);
}

QString InterfaceActiveVault::getEncryptDir()
{
    return OperatorCenter::getInstance()->getEncryptDirPath();
}

QString InterfaceActiveVault::getDecryptDir()
{
    return OperatorCenter::getInstance()->getdecryptDirPath();
}

QStringList InterfaceActiveVault::getConfigFilePath()
{
    return OperatorCenter::getInstance()->getConfigFilePath();
}

EN_VaultState InterfaceActiveVault::vaultState()
{
    return OperatorCenter::getInstance()->vaultState();
}

bool InterfaceActiveVault::getRootPassword()
{
    return OperatorCenter::getInstance()->getRootPassword();
}

int InterfaceActiveVault::executionShellCommand(const QString &strCmd, QStringList &lstShellOutput)
{
    return OperatorCenter::getInstance()->executionShellCommand(strCmd, lstShellOutput);
}
