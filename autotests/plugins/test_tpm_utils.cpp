// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_tpm_utils.cpp
 * @brief Unit tests for tpm_utils methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "utils/encryptutils.h"

#include <QTest>

using namespace dfmplugin_disk_encrypt_entry;

class tpm_utilsTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new tpm_utils();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    tpm_utils *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(tpm_utilsTest, checkTPM)
{
    // Test getter: int checkTPM()
    auto result = obj->checkTPM();
    EXPECT_EQ(result, 0);

}

TEST_F(tpm_utilsTest, checkTPMLockoutStatus)
{
    // Test getter: int checkTPMLockoutStatus()
    auto result = obj->checkTPMLockoutStatus();
    EXPECT_EQ(result, 0);

}

TEST_F(tpm_utilsTest, encryptByTPM)
{
    // Test method: int encryptByTPM((const QVariantMap &map))
    QVariantMap _arg0{};
    auto result = obj->encryptByTPM(_arg0);
    EXPECT_GE(result, 0);

}

TEST_F(tpm_utilsTest, isSupportAlgoByTPM)
{
    // Test method: int isSupportAlgoByTPM((const QString &algoName, bool *support))
    QString _arg0{};
    auto result = obj->isSupportAlgoByTPM(_arg0, nullptr);
    EXPECT_GE(result, 0);

}

TEST_F(tpm_utilsTest, ownerAuthStatus)
{
    // Test getter: int ownerAuthStatus()
    auto result = obj->ownerAuthStatus();
    EXPECT_EQ(result, 0);

}
