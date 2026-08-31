// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_deviceproxymanager.cpp
 * @brief Unit tests for DeviceProxyManager methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "dfm-base/base/device/deviceproxymanager.h"

#include <QTest>

using namespace src;

class DeviceProxyManagerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new DeviceProxyManager();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    DeviceProxyManager *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(DeviceProxyManagerTest, getAllBlockIdsByUUID)
{
    // Test method: QStringList getAllBlockIdsByUUID((const QStringList &uuids, GlobalServerDefines::DeviceQueryOptions opts))
    QStringList _arg0{};
    auto result = obj->getAllBlockIdsByUUID(_arg0, GlobalServerDefines::DeviceQueryOptions());
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(DeviceProxyManagerTest, isMptOfDevice)
{
    // Test method: bool isMptOfDevice((const QString &filePath, QString &id))
    QString _arg0{};
    QString _arg1{};
    auto result = obj->isMptOfDevice(_arg0, _arg1);
    EXPECT_FALSE(result);

}

TEST_F(DeviceProxyManagerTest, queryDeviceInfoByPath)
{
    // Test method: QVariantMap queryDeviceInfoByPath((const QString &path, bool reload))
    QString _arg0{};
    auto result = obj->queryDeviceInfoByPath(_arg0, false);
    EXPECT_TRUE(result.isEmpty());

}
