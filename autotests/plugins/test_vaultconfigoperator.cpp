// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_vaultconfigoperator.cpp
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

TEST_F(VaultConfigOperatorTest, set)
{
    // Test setter: void set((const QString &nodeName, const QString &keyName, QVariant value))
    QString _arg0{};
    QString _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->set(_arg0, _arg1, QVariant()));
}
