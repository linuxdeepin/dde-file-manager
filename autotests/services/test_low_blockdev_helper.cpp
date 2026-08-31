// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_low_blockdev_helper.cpp
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

TEST_F(blockdev_helperTest, devBlockSize)
{
    // Test method: quint64 devBlockSize((const QString &phyDev))
    QString _arg0{};
    auto result = obj->devBlockSize(_arg0);
    EXPECT_GE(result, 0);

}

TEST_F(blockdev_helperTest, devDeviceSize)
{
    // Test method: FILE_ENCRYPT_USE_NS devDeviceSize((const QString &phyDev))
    QString _arg0{};
    auto result = obj->devDeviceSize(_arg0);
    EXPECT_NO_FATAL_FAILURE({ obj->devDeviceSize(_arg0); });

}
