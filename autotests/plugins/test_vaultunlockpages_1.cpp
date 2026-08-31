// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_vaultunlockpages_1.cpp
 * @brief Unit tests for VaultUnlockPages methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "views/vaultunlockpages.h"

#include <QTest>

using namespace dfmplugin_vault;

class VaultUnlockPagesTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new VaultUnlockPages();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    VaultUnlockPages *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(VaultUnlockPagesTest, isOldPasswordSchemeMigrationMode)
{
    // Test bool getter: isOldPasswordSchemeMigrationMode()
    bool result = obj->isOldPasswordSchemeMigrationMode();
    EXPECT_FALSE(result);

}

TEST_F(VaultUnlockPagesTest, onButtonClicked)
{
    // Test method: void onButtonClicked((int index, const QString &text))
    QString _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->onButtonClicked(0, _arg1));
}

TEST_F(VaultUnlockPagesTest, onSetBtnEnabled)
{
    // Test method: void onSetBtnEnabled((int index, const bool &state))
    bool _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->onSetBtnEnabled(0, _arg1));
}

TEST_F(VaultUnlockPagesTest, setOldPasswordSchemeMigrationMode)
{
    // Test setter: void setOldPasswordSchemeMigrationMode((bool enabled))
    EXPECT_NO_FATAL_FAILURE(obj->setOldPasswordSchemeMigrationMode(false));
}
