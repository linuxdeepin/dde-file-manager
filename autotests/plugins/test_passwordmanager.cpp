// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_passwordmanager.cpp
 * @brief Unit tests for PasswordManager methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "utils/encryption/passwordmanager.h"

#include <QTest>

using namespace dfmplugin_vault;

class PasswordManagerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new PasswordManager();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    PasswordManager *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(PasswordManagerTest, deleteKeyslot)
{
    // Test method: int deleteKeyslot((const char *path,
                                    int keyslot))
    auto result = obj->deleteKeyslot(nullptr, 0);
    EXPECT_GE(result, 0);

}
