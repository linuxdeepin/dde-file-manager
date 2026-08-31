// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_vaultconfigoperator_1.cpp
 * @brief Unit tests for VaultConfigOperator methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "vaultconfigoperator.h"

#include <QTest>

using namespace vault;

class VaultConfigOperatorTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new VaultConfigOperator();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    VaultConfigOperator *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(VaultConfigOperatorTest, get)
{
    // Test method: QVariant get((const QString &nodeName, const QString &keyName, const QVariant &defaultValue))
    QString _arg0{};
    QString _arg1{};
    QVariant _arg2{};
    auto result = obj->get(_arg0, _arg1, _arg2);
    EXPECT_FALSE(result.isValid());

}

TEST_F(VaultConfigOperatorTest, VaultConfigOperator_Destructor)
{
    // Test method:  ~VaultConfigOperator(())
    EXPECT_NO_FATAL_FAILURE({ VaultConfigOperator *tmp = new VaultConfigOperator(); delete tmp; });
}
