// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_protocoldevicedisplaymanagerprivate_1.cpp
 * @brief Unit tests for ProtocolDeviceDisplayManagerPrivate methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "displaycontrol/protocoldevicedisplaymanager_p.h"

#include <QTest>

using namespace dfmplugin_smbbrowser;

class ProtocolDeviceDisplayManagerPrivateTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new ProtocolDeviceDisplayManagerPrivate();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    ProtocolDeviceDisplayManagerPrivate *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(ProtocolDeviceDisplayManagerPrivateTest, ProtocolDeviceDisplayManagerPrivate)
{
    // Test constructor: ProtocolDeviceDisplayManagerPrivate(())
    ASSERT_NE(obj, nullptr);
}

TEST_F(ProtocolDeviceDisplayManagerPrivateTest, init)
{
    // Test method: void init(())
    EXPECT_NO_FATAL_FAILURE(obj->init());
}

TEST_F(ProtocolDeviceDisplayManagerPrivateTest, isSupportVEntry)
{
    // Test method: bool isSupportVEntry((const QString &devId))
    QString _arg0{};
    auto result = obj->isSupportVEntry(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(ProtocolDeviceDisplayManagerPrivateTest, onDisplayModeChanged)
{
    // Test method: void onDisplayModeChanged(())
    EXPECT_NO_FATAL_FAILURE(obj->onDisplayModeChanged());
}

TEST_F(ProtocolDeviceDisplayManagerPrivateTest, onShowOfflineChanged)
{
    // Test method: void onShowOfflineChanged(())
    EXPECT_NO_FATAL_FAILURE(obj->onShowOfflineChanged());
}

TEST_F(ProtocolDeviceDisplayManagerPrivateTest, ProtocolDeviceDisplayManagerPrivate_Destructor)
{
    // Test method:  ~ProtocolDeviceDisplayManagerPrivate(())
    EXPECT_NO_FATAL_FAILURE({ ProtocolDeviceDisplayManagerPrivate *tmp = new ProtocolDeviceDisplayManagerPrivate(); delete tmp; });
}
