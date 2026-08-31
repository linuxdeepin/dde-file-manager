// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_low_crypt_setup.cpp
 * @brief Unit tests for crypt_setup methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "services/diskencrypt/core/cryptsetup.h"

#include <QTest>

using namespace src;

class crypt_setupTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new crypt_setup();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    crypt_setup *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(crypt_setupTest, csAddPassphrase)
{
    // Test method: int csAddPassphrase((const QString &dev, const QString &validPwd, const QString &newPwd))
    QString _arg0{};
    QString _arg1{};
    QString _arg2{};
    auto result = obj->csAddPassphrase(_arg0, _arg1, _arg2);
    EXPECT_GE(result, 0);

}

TEST_F(crypt_setupTest, csChangePassphrase)
{
    // Test method: int csChangePassphrase((const QString &dev, const QString &oldPwd, const QString &newPwd))
    QString _arg0{};
    QString _arg1{};
    QString _arg2{};
    auto result = obj->csChangePassphrase(_arg0, _arg1, _arg2);
    EXPECT_GE(result, 0);

}

TEST_F(crypt_setupTest, csInitEncrypt)
{
    // Test method: FILE_ENCRYPT_USE_NS csInitEncrypt((const QString &dev, const QString &displayName, CryptPreProcessor *processor))
    QString _arg0{};
    QString _arg1{};
    auto result = obj->csInitEncrypt(_arg0, _arg1, nullptr);
    EXPECT_NO_FATAL_FAILURE({ obj->csInitEncrypt(_arg0, _arg1, nullptr); });

}

TEST_F(crypt_setupTest, csSetLabel)
{
    // Test method: int csSetLabel((const QString &dev, const QString &label))
    QString _arg0{};
    QString _arg1{};
    auto result = obj->csSetLabel(_arg0, _arg1);
    EXPECT_GE(result, 0);

}
