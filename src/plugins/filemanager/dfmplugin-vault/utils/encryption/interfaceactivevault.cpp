// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "interfaceactivevault.h"
#include "operatorcenter.h"
#include "utils/vaulthelper.h"
#include "utils/pathmanager.h"

using namespace dfmplugin_vault;

InterfaceActiveVault::InterfaceActiveVault(QObject *parent)
    : QObject(parent)
{
}

InterfaceActiveVault::~InterfaceActiveVault()
{
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

VaultState InterfaceActiveVault::vaultState()
{
    return VaultHelper::instance()->state(PathManager::makeVaultLocalPath(QString(""), kVaultEncrypyDirName));
}

bool InterfaceActiveVault::getRootPassword()
{
    return OperatorCenter::getInstance()->getRootPassword();
}

int InterfaceActiveVault::executionShellCommand(const QString &strCmd, QStringList &lstShellOutput)
{
    return OperatorCenter::getInstance()->executionShellCommand(strCmd, lstShellOutput);
}
