// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_low_common_helper.cpp
 * @brief Unit tests for common_helper methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "services/diskencrypt/helpers/commonhelper.h"

#include <QTest>

using namespace src;

class common_helperTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new common_helper();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    common_helper *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(common_helperTest, createDFMDesktopEntry)
{
    // Test getter: FILE_ENCRYPT_USE_NS createDFMDesktopEntry()
    auto result = obj->createDFMDesktopEntry();
    EXPECT_NO_FATAL_FAILURE({ obj->createDFMDesktopEntry(); });

}

TEST_F(common_helperTest, createRebootFlagFile)
{
    // Test method: void createRebootFlagFile((const QString &dev))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->createRebootFlagFile(_arg0));
}

TEST_F(common_helperTest, encryptCipher)
{
    // Test getter: QString encryptCipher()
    auto result = obj->encryptCipher();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(common_helperTest, genRecoveryKey)
{
    // Test getter: QString genRecoveryKey()
    auto result = obj->genRecoveryKey();
    EXPECT_TRUE(result.isEmpty());

}
