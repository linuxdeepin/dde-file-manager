// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_crypt_setup_helper.cpp
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

TEST_F(crypt_setup_helperTest, backupDetachHeader)
{
    // Test method: int backupDetachHeader((const QString &dev, QString *fileHeader))
    QString _arg0{};
    auto result = obj->backupDetachHeader(_arg0, nullptr);
    EXPECT_GE(result, 0);

}

TEST_F(crypt_setup_helperTest, genDetachHeaderPath)
{
    // Test method: int genDetachHeaderPath((const QString &dev, QString *name))
    QString _arg0{};
    auto result = obj->genDetachHeaderPath(_arg0, nullptr);
    EXPECT_GE(result, 0);

}

TEST_F(crypt_setup_helperTest, getToken)
{
    // Test method: int getToken((const QString &dev, QString *token))
    QString _arg0{};
    auto result = obj->getToken(_arg0, nullptr);
    EXPECT_GE(result, 0);

}

TEST_F(crypt_setup_helperTest, setToken)
{
    // Test method: int setToken((const QString &dev, const QString &token))
    QString _arg0{};
    QString _arg1{};
    auto result = obj->setToken(_arg0, _arg1);
    EXPECT_GE(result, 0);

}
