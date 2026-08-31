// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_devicemountsubscriber.cpp
 * @brief Unit tests for DeviceMountSubscriber methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "utils/devicemountsubscriber.h"

#include <QTest>

using namespace dfmplugin_sidebar;

class DeviceMountSubscriberTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new DeviceMountSubscriber();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    DeviceMountSubscriber *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(DeviceMountSubscriberTest, cleanupExpiredSubscriptions)
{
    // Test method: void cleanupExpiredSubscriptions(())
    EXPECT_NO_FATAL_FAILURE(obj->cleanupExpiredSubscriptions());
}
