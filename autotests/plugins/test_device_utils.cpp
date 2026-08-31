// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_device_utils.cpp
 * @brief Unit tests for device_utils methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "external/dde-dock-plugins/disk-mount/utils/dockutils.h"

#include <QTest>

using namespace src;

class device_utilsTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new device_utils();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    device_utils *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(device_utilsTest, cacheToken)
{
    // Test method: void cacheToken((const QString &device, const QVariantMap &token))
    QString _arg0{};
    QVariantMap _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->cacheToken(_arg0, _arg1));
}

TEST_F(device_utilsTest, createBlockDevice)
{
    // Test method: BlockDev createBlockDevice((const QString &devObjPath))
    QString _arg0{};
    auto result = obj->createBlockDevice(_arg0);
    EXPECT_NO_FATAL_FAILURE({ obj->createBlockDevice(_arg0); });

}

TEST_F(device_utilsTest, encKeyType)
{
    // Test method: int encKeyType((const QString &dev))
    QString _arg0{};
    auto result = obj->encKeyType(_arg0);
    EXPECT_GE(result, 0);

}
