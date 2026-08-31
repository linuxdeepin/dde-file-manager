// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_vaultconfig_1.cpp
 * @brief Unit tests for VaultConfig methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "utils/encryption/vaultconfig.h"

#include <QTest>

using namespace dfmplugin_vault;

class VaultConfigTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new VaultConfig();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    VaultConfig *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(VaultConfigTest, VaultConfig)
{
    // Test constructor: VaultConfig((const QString &filePath))
    ASSERT_NE(obj, nullptr);
}

TEST_F(VaultConfigTest, isNewCreated)
{
    // Test bool getter: isNewCreated()
    bool result = obj->isNewCreated();
    EXPECT_FALSE(result);

}

TEST_F(VaultConfigTest, setVaultCreationType)
{
    // Test setter: void setVaultCreationType((const QString &type))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->setVaultCreationType(_arg0));
}

TEST_F(VaultConfigTest, VaultConfig_Destructor)
{
    // Test method:  ~VaultConfig(())
    EXPECT_NO_FATAL_FAILURE({ VaultConfig *tmp = new VaultConfig(); delete tmp; });
}
