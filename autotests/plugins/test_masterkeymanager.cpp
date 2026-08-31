// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_masterkeymanager.cpp
 * @brief Unit tests for MasterKeyManager methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "utils/encryption/masterkeymanager.h"

#include <QTest>

using namespace dfmplugin_vault;

class MasterKeyManagerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new MasterKeyManager();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    MasterKeyManager *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(MasterKeyManagerTest, generateMasterKey)
{
    // Test getter: QByteArray generateMasterKey()
    auto result = obj->generateMasterKey();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(MasterKeyManagerTest, generateMasterKeyFromPassword)
{
    // Test method: QByteArray generateMasterKeyFromPassword((const QString &password))
    QString _arg0{};
    auto result = obj->generateMasterKeyFromPassword(_arg0);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(MasterKeyManagerTest, getContainerPath)
{
    // Test getter: QString getContainerPath()
    auto result = obj->getContainerPath();
    EXPECT_TRUE(result.isEmpty());

}
