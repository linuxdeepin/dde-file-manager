// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_blockdev_helper.cpp
 * @brief Unit tests for blockdev_helper methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "services/diskencrypt/helpers/blockdevhelper.h"

#include <QTest>

using namespace src;

class blockdev_helperTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new blockdev_helper();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    blockdev_helper *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(blockdev_helperTest, createDevPtr)
{
    // Test method: DevPtr createDevPtr((const QString &dev))
    QString _arg0{};
    auto result = obj->createDevPtr(_arg0);
    EXPECT_NE(result.get(), nullptr);

}

TEST_F(blockdev_helperTest, createDevPtr2)
{
    // Test method: DevPtr createDevPtr2((const QString &objPath))
    QString _arg0{};
    auto result = obj->createDevPtr2(_arg0);
    EXPECT_NE(result.get(), nullptr);

}

TEST_F(blockdev_helperTest, devCryptVersion)
{
    // Test method: int devCryptVersion((const QString &dev))
    QString _arg0{};
    auto result = obj->devCryptVersion(_arg0);
    EXPECT_GE(result, 0);

}

TEST_F(blockdev_helperTest, getUSecName)
{
    // Test method: QString getUSecName((const QString &dmDev))
    QString _arg0{};
    auto result = obj->getUSecName(_arg0);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(blockdev_helperTest, resolveDevObjPath)
{
    // Test method: QString resolveDevObjPath((const QString &source))
    QString _arg0{};
    auto result = obj->resolveDevObjPath(_arg0);
    EXPECT_TRUE(result.isEmpty());

}
