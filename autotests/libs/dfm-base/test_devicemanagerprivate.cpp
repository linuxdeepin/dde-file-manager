// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_devicemanagerprivate.cpp
 * @brief Unit tests for DeviceManagerPrivate methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "dfm-base/base/device/devicemanager.h"

#include <QTest>

using namespace src;

class DeviceManagerPrivateTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new DeviceManagerPrivate();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    DeviceManagerPrivate *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(DeviceManagerPrivateTest, shouldAutoMountBlockDevice)
{
    // Test method: bool shouldAutoMountBlockDevice((const QString &id, const QVariantMap &info))
    QString _arg0{};
    QVariantMap _arg1{};
    auto result = obj->shouldAutoMountBlockDevice(_arg0, _arg1);
    EXPECT_FALSE(result);

}
