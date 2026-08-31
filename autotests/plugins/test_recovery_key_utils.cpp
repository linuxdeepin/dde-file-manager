// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_recovery_key_utils.cpp
 * @brief Unit tests for recovery_key_utils methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "utils/encryptutils.h"

#include <QTest>

using namespace dfmplugin_disk_encrypt_entry;

class recovery_key_utilsTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new recovery_key_utils();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    recovery_key_utils *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(recovery_key_utilsTest, formatRecoveryKey)
{
    // Test method: QString formatRecoveryKey((const QString &raw))
    QString _arg0{};
    auto result = obj->formatRecoveryKey(_arg0);
    EXPECT_TRUE(result.isEmpty());

}
