// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "utils/encryption/interfaceactivevault.h"
#include "utils/encryption/operatorcenter.h"
#include "utils/vaulthelper.h"

#include <gtest/gtest.h>

DPVAULT_USE_NAMESPACE

TEST(UT_InterfaceActiveVault, getPasswordHint)
{
    bool isOk { false };

    stub_ext::StubExt stub;
    stub.set_lamda(&OperatorCenter::getPasswordHint, [ &isOk ]{
        isOk = true;
        return true;
    });

    InterfaceActiveVault inter;
    QString hint;
    inter.getPasswordHint(hint);

    EXPECT_TRUE(isOk);
}

TEST(UT_InterfaceActiveVault, checkPassword)
{
    bool isOk { false };

    stub_ext::StubExt stub;
    stub.set_lamda(&OperatorCenter::checkPassword, [ &isOk ]{
        isOk = true;
        return true;
    });

    InterfaceActiveVault inter;
    QString cipher;
    inter.checkPassword("UT_TEST", cipher);

    EXPECT_TRUE(isOk);
}

TEST(UT_InterfaceActiveVault, checkUserKey)
{
    bool isOk { false };

    stub_ext::StubExt stub;
    stub.set_lamda(&OperatorCenter::checkUserKey, [ &isOk ]{
        isOk = true;
        return true;
    });

    InterfaceActiveVault inter;
    QString cipher;
    inter.checkUserKey("UT_TEST", cipher);

    EXPECT_TRUE(isOk);
}

TEST(UT_InterfaceActiveVault, getEncryptDir)
{
    bool isOk { false };

    stub_ext::StubExt stub;
    stub.set_lamda(&OperatorCenter::getEncryptDirPath, [ &isOk ]{
        isOk = true;
        return "";
    });

    InterfaceActiveVault inter;
    inter.getEncryptDir();

    EXPECT_TRUE(isOk);
}

TEST(UT_InterfaceActiveVault, getDecryptDir)
{
    bool isOk { false };

    stub_ext::StubExt stub;
    stub.set_lamda(&OperatorCenter::getdecryptDirPath, [ &isOk ]{
        isOk = true;
        return "";
    });

    InterfaceActiveVault inter;
    inter.getDecryptDir();

    EXPECT_TRUE(isOk);
}

TEST(UT_InterfaceActiveVault, getConfigFilePath)
{
    bool isOk { false };

    stub_ext::StubExt stub;
    stub.set_lamda(&OperatorCenter::getConfigFilePath, [ &isOk ]{
        isOk = true;
        return QStringList();
    });

    InterfaceActiveVault inter;
    inter.getConfigFilePath();

    EXPECT_TRUE(isOk);
}

TEST(UT_InterfaceActiveVault, vaultState)
{
    bool isOk { false };

    stub_ext::StubExt stub;
    stub.set_lamda(&VaultHelper::state, [ &isOk ]{
        isOk = true;
        return VaultState::kUnknow;
    });

    InterfaceActiveVault inter;
    inter.vaultState();

    EXPECT_TRUE(isOk);
}

TEST(UT_InterfaceActiveVault, getRootPassword)
{
    bool isOk { false };

    stub_ext::StubExt stub;
    stub.set_lamda(&OperatorCenter::getRootPassword, [ &isOk ]{
        isOk = true;
        return true;
    });

    InterfaceActiveVault inter;
    inter.getRootPassword();

    EXPECT_TRUE(isOk);
}

TEST(UT_InterfaceActiveVault, executionShellCommand)
{
    bool isOk { false };

    stub_ext::StubExt stub;
    stub.set_lamda(&OperatorCenter::executionShellCommand, [ &isOk ]{
        isOk = true;
        return 0;
    });

    InterfaceActiveVault inter;
    QStringList output;
    inter.executionShellCommand("UT_TEST", output);

    EXPECT_TRUE(isOk);
}
