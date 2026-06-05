// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "interfaceactivevault.h"
#include "operatorcenter.h"

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
