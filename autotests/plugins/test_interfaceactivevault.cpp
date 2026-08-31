// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_interfaceactivevault.cpp
 * @brief Unit tests for InterfaceActiveVault methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "utils/encryption/interfaceactivevault.h"

#include <QTest>

using namespace dfmplugin_vault;

class InterfaceActiveVaultTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new InterfaceActiveVault();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    InterfaceActiveVault *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(InterfaceActiveVaultTest, InterfaceActiveVault)
{
    // Test constructor: InterfaceActiveVault((QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(InterfaceActiveVaultTest, checkPassword)
{
    // Test method: bool checkPassword((const QString &password, QString &cipher))
    QString _arg0{};
    QString _arg1{};
    auto result = obj->checkPassword(_arg0, _arg1);
    EXPECT_FALSE(result);

}

TEST_F(InterfaceActiveVaultTest, checkUserKey)
{
    // Test method: bool checkUserKey((const QString &userKey, QString &cipher))
    QString _arg0{};
    QString _arg1{};
    auto result = obj->checkUserKey(_arg0, _arg1);
    EXPECT_FALSE(result);

}

TEST_F(InterfaceActiveVaultTest, getPasswordHint)
{
    // Test method: bool getPasswordHint((QString &passwordHint))
    QString _arg0{};
    auto result = obj->getPasswordHint(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(InterfaceActiveVaultTest, InterfaceActiveVault_Destructor)
{
    // Test method:  ~InterfaceActiveVault(())
    EXPECT_NO_FATAL_FAILURE({ InterfaceActiveVault *tmp = new InterfaceActiveVault(); delete tmp; });
}
