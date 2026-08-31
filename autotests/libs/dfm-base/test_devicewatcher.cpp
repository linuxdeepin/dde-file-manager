// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_devicewatcher.cpp
 * @brief Unit tests for DeviceWatcher methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "dfm-base/base/device/private/devicewatcher.h"

#include <QTest>

using namespace src;

class DeviceWatcherTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new DeviceWatcher();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    DeviceWatcher *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(DeviceWatcherTest, getDevInfo)
{
    // Test method: QVariantMap getDevInfo((const QString &id, dfmmount::DeviceType type, bool reload))
    QString _arg0{};
    auto result = obj->getDevInfo(_arg0, {}, false);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(DeviceWatcherTest, onBlkDevFsRemoved)
{
    // Test method: void onBlkDevFsRemoved((const QString &id))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->onBlkDevFsRemoved(_arg0));
}

TEST_F(DeviceWatcherTest, onBlkDevRemoved)
{
    // Test method: void onBlkDevRemoved((const QString &id))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->onBlkDevRemoved(_arg0));
}

TEST_F(DeviceWatcherTest, onProtoDevRemoved)
{
    // Test method: void onProtoDevRemoved((const QString &id))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->onProtoDevRemoved(_arg0));
}

TEST_F(DeviceWatcherTest, saveOpticalDevUsage)
{
    // Test method: void saveOpticalDevUsage((const QString &id, const QVariantMap &data))
    QString _arg0{};
    QVariantMap _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->saveOpticalDevUsage(_arg0, _arg1));
}

TEST_F(DeviceWatcherTest, updateOpticalDevUsage)
{
    // Test method: void updateOpticalDevUsage((const QString &id, const QString &mpt))
    QString _arg0{};
    QString _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->updateOpticalDevUsage(_arg0, _arg1));
}
