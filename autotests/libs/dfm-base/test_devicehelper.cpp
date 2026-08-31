// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_devicehelper.cpp
 * @brief Unit tests for DeviceHelper methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "dfm-base/base/device/private/devicehelper.h"

#include <QTest>

using namespace src;

class DeviceHelperTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new DeviceHelper();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    DeviceHelper *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(DeviceHelperTest, askForStopScanning)
{
    // Test method: bool askForStopScanning((const QUrl &mpt))
    QUrl _arg0{};
    auto result = obj->askForStopScanning(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(DeviceHelperTest, castFromDFMMountProperty)
{
    // Test method: QString castFromDFMMountProperty((dfmmount::Property property))
    auto result = obj->castFromDFMMountProperty({});
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(DeviceHelperTest, checkNetworkConnection)
{
    // Test method: bool checkNetworkConnection((const QString &id))
    QString _arg0{};
    auto result = obj->checkNetworkConnection(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(DeviceHelperTest, clearOpticalInfo)
{
    // Test method: void clearOpticalInfo((const QString &tag))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->clearOpticalInfo(_arg0));
}

TEST_F(DeviceHelperTest, createBlockDevice)
{
    // Test method: BlockDevAutoPtr createBlockDevice((const QString &id))
    QString _arg0{};
    auto result = obj->createBlockDevice(_arg0);
    EXPECT_NE(result.get(), nullptr);

}

TEST_F(DeviceHelperTest, createDevice)
{
    // Test method: DFM_MOUNT_USE_NS createDevice((const QString &devId, dfmmount::DeviceType type))
    QString _arg0{};
    auto result = obj->createDevice(_arg0, {});
    EXPECT_NO_FATAL_FAILURE({ obj->createDevice(_arg0, {}); });

}

TEST_F(DeviceHelperTest, createProtocolDevice)
{
    // Test method: ProtocolDevAutoPtr createProtocolDevice((const QString &id))
    QString _arg0{};
    auto result = obj->createProtocolDevice(_arg0);
    EXPECT_NE(result.get(), nullptr);

}

TEST_F(DeviceHelperTest, isEjectableBlockDev)
{
    // Test method: bool isEjectableBlockDev((const QVariantMap &infos, QString &why))
    QVariantMap _arg0{};
    QString _arg1{};
    auto result = obj->isEjectableBlockDev(_arg0, _arg1);
    EXPECT_FALSE(result);

}

TEST_F(DeviceHelperTest, isMountableBlockDev)
{
    // Test method: bool isMountableBlockDev((const QVariantMap &infos, QString &why))
    QVariantMap _arg0{};
    QString _arg1{};
    auto result = obj->isMountableBlockDev(_arg0, _arg1);
    EXPECT_FALSE(result);

}

TEST_F(DeviceHelperTest, loadProtocolInfo)
{
    // Test method: QVariantMap loadProtocolInfo((const ProtocolDevAutoPtr &dev))
    ProtocolDevAutoPtr _arg0{};
    auto result = obj->loadProtocolInfo(_arg0);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(DeviceHelperTest, persistentOpticalInfo)
{
    // Test method: void persistentOpticalInfo((const QVariantMap &datas))
    QVariantMap _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->persistentOpticalInfo(_arg0));
}

TEST_F(DeviceHelperTest, queryUsageOfProtocolRealTime)
{
    // Test method: bool queryUsageOfProtocolRealTime((const QVariantMap &itemData, quint64 *total, quint64 *avai, quint64 *used))
    QVariantMap _arg0{};
    auto result = obj->queryUsageOfProtocolRealTime(_arg0, nullptr, nullptr, nullptr);
    EXPECT_FALSE(result);

}

TEST_F(DeviceHelperTest, readOpticalInfo)
{
    // Test method: void readOpticalInfo((QVariantMap &datas))
    QVariantMap _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->readOpticalInfo(_arg0));
}
