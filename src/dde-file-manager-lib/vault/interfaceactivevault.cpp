// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
