// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_low_crypt_setup_helper.cpp
 * @brief Unit tests for crypt_setup_helper methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "services/diskencrypt/core/cryptsetup.h"

#include <QTest>

using namespace src;

class crypt_setup_helperTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new crypt_setup_helper();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    crypt_setup_helper *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(crypt_setup_helperTest, createHeaderFile)
{
    // Test method: int createHeaderFile((const QString &dev, QString *headerPath))
    QString _arg0{};
    auto result = obj->createHeaderFile(_arg0, nullptr);
    EXPECT_GE(result, 0);

}

TEST_F(crypt_setup_helperTest, initDeviceHeader)
{
    // Test method: int initDeviceHeader((const QString &dev, const QString &fileHeader))
    QString _arg0{};
    QString _arg1{};
    auto result = obj->initDeviceHeader(_arg0, _arg1);
    EXPECT_GE(result, 0);

}

TEST_F(crypt_setup_helperTest, initiable)
{
    // Test method: int initiable((const QString &dev))
    QString _arg0{};
    auto result = obj->initiable(_arg0);
    EXPECT_GE(result, 0);

}

TEST_F(crypt_setup_helperTest, onDecrypting)
{
    // Test method: int onDecrypting((uint64_t size, uint64_t offset, void *usrptr))
    auto result = obj->onDecrypting({}, {}, nullptr);
    EXPECT_GE(result, 0);

}

TEST_F(crypt_setup_helperTest, onEncrypting)
{
    // Test method: int onEncrypting((uint64_t size, uint64_t offset, void *usrptr))
    auto result = obj->onEncrypting({}, {}, nullptr);
    EXPECT_GE(result, 0);

}
