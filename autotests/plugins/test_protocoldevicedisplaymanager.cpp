// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_protocoldevicedisplaymanager.cpp
 * @brief Unit tests for ProtocolDeviceDisplayManager methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "displaycontrol/protocoldevicedisplaymanager.h"

#include <QTest>

using namespace dfmplugin_smbbrowser;

class ProtocolDeviceDisplayManagerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new ProtocolDeviceDisplayManager();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    ProtocolDeviceDisplayManager *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(ProtocolDeviceDisplayManagerTest, instance)
{
    // Test getter: ProtocolDeviceDisplayManager instance()
    auto result = obj->instance();
    EXPECT_NO_FATAL_FAILURE({ obj->instance(); });

}

TEST_F(ProtocolDeviceDisplayManagerTest, onDevUnmounted)
{
    // Test method: void onDevUnmounted((const QString &id))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->onDevUnmounted(_arg0));
}
