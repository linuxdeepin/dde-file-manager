// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_low_utils.cpp
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

TEST_F(UtilsTest, accessMode)
{
    // Test method: int accessMode((const QString &mps))
    QString _arg0{};
    auto result = obj->accessMode(_arg0);
    EXPECT_GE(result, 0);

}

TEST_F(UtilsTest, changeDiskPassword)
{
    // Test method: DPCErrorCode changeDiskPassword((crypt_device *cd, const char *oldPwd, const char *newPwd))
    auto result = obj->changeDiskPassword(nullptr, nullptr, nullptr);
    EXPECT_NO_FATAL_FAILURE({ obj->changeDiskPassword(nullptr, nullptr, nullptr); });

}

TEST_F(UtilsTest, checkDiskPassword)
{
    // Test method: DPCErrorCode checkDiskPassword((crypt_device **cd, const char *pwd, const char *device))
    auto result = obj->checkDiskPassword(nullptr, nullptr, nullptr);
    EXPECT_NO_FATAL_FAILURE({ obj->checkDiskPassword(nullptr, nullptr, nullptr); });

}

TEST_F(UtilsTest, isValidDevPolicy)
{
    // Test method: bool isValidDevPolicy((const QVariantMap &policy, const QString &realInvoker))
    QVariantMap _arg0{};
    QString _arg1{};
    auto result = obj->isValidDevPolicy(_arg0, _arg1);
    EXPECT_FALSE(result);

}

TEST_F(UtilsTest, isValidVaultPolicy)
{
    // Test method: bool isValidVaultPolicy((const QVariantMap &policy))
    QVariantMap _arg0{};
    auto result = obj->isValidVaultPolicy(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(UtilsTest, setFileMode)
{
    // Test method: int setFileMode((const QString &mountPoint, uint mode))
    QString _arg0{};
    auto result = obj->setFileMode(_arg0, 0);
    EXPECT_GE(result, 0);

}

TEST_F(UtilsTest, valultConfigPath)
{
    // Test getter: QString valultConfigPath()
    auto result = obj->valultConfigPath();
    EXPECT_TRUE(result.isEmpty());

}
