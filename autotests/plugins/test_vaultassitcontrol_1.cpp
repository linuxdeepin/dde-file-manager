// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_vaultassitcontrol_1.cpp
 * @brief Unit tests for VaultAssitControl methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "vaultassist/vaultassitcontrol.h"

#include <QTest>

using namespace dfmplugin_utils;

class VaultAssitControlTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new VaultAssitControl();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    VaultAssitControl *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(VaultAssitControlTest, buildVaultLocalPath)
{
    // Test method: QString buildVaultLocalPath((const QString &path, const QString &base))
    QString _arg0{};
    QString _arg1{};
    auto result = obj->buildVaultLocalPath(_arg0, _arg1);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(VaultAssitControlTest, instance)
{
    // Test getter: VaultAssitControl instance()
    auto result = obj->instance();
    EXPECT_NO_FATAL_FAILURE({ obj->instance(); });

}

TEST_F(VaultAssitControlTest, isVaultFile)
{
    // Test method: bool isVaultFile((const QUrl &url))
    QUrl _arg0{};
    auto result = obj->isVaultFile(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(VaultAssitControlTest, vaultBaseDirLocalPath)
{
    // Test getter: QString vaultBaseDirLocalPath()
    auto result = obj->vaultBaseDirLocalPath();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(VaultAssitControlTest, vaultMountDirLocalPath)
{
    // Test getter: QString vaultMountDirLocalPath()
    auto result = obj->vaultMountDirLocalPath();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(VaultAssitControlTest, vaultUrlToLocalUrl)
{
    // Test method: QUrl vaultUrlToLocalUrl((const QUrl &url))
    QUrl _arg0{};
    auto result = obj->vaultUrlToLocalUrl(_arg0);
    EXPECT_FALSE(result.isValid());

}
