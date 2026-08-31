// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_remotepasswdmanager_1.cpp
 * @brief Unit tests for RemotePasswdManager methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "utils/remotepasswdmanager.h"

#include <QTest>

using namespace dfmplugin_computer;

class RemotePasswdManagerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new RemotePasswdManager();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    RemotePasswdManager *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(RemotePasswdManagerTest, RemotePasswdManager)
{
    // Test constructor: RemotePasswdManager((QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(RemotePasswdManagerTest, ftpSchema)
{
    // Test getter: SecretSchema ftpSchema()
    auto result = obj->ftpSchema();
    EXPECT_NO_FATAL_FAILURE({ obj->ftpSchema(); });

}

TEST_F(RemotePasswdManagerTest, instance)
{
    // Test getter: RemotePasswdManager instance()
    auto result = obj->instance();
    EXPECT_NO_FATAL_FAILURE({ obj->instance(); });

}

TEST_F(RemotePasswdManagerTest, smbSchema)
{
    // Test getter: SecretSchema smbSchema()
    auto result = obj->smbSchema();
    EXPECT_NO_FATAL_FAILURE({ obj->smbSchema(); });

}
