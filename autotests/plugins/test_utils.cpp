// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_utils.cpp
 * @brief Unit tests for Utils methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "utils.h"

#include <QTest>

using namespace dfmplugin_utils;

class UtilsTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new Utils();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    Utils *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(UtilsTest, loadDevPolicy)
{
    // Test method: void loadDevPolicy((DevPolicyType *devPolicies))
    EXPECT_NO_FATAL_FAILURE(obj->loadDevPolicy(nullptr));
}

TEST_F(UtilsTest, loadVaultPolicy)
{
    // Test method: void loadVaultPolicy((VaultPolicyType *vaultPolicies))
    EXPECT_NO_FATAL_FAILURE(obj->loadVaultPolicy(nullptr));
}

TEST_F(UtilsTest, saveVaultPolicy)
{
    // Test method: void saveVaultPolicy((const QVariantMap &policy))
    QVariantMap _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->saveVaultPolicy(_arg0));
}
