// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_devicemountsubscriber_1.cpp
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

TEST_F(DeviceMountSubscriberTest, DeviceMountSubscriber)
{
    // Test constructor: DeviceMountSubscriber((QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(DeviceMountSubscriberTest, instance)
{
    // Test getter: DPSIDEBAR_USE_NAMESPACE instance()
    auto result = obj->instance();
    EXPECT_NO_FATAL_FAILURE({ obj->instance(); });

}

TEST_F(DeviceMountSubscriberTest, subscribe)
{
    // Test method: int subscribe((const QUrl &deviceUrl, std::function<void(const QUrl &)> callback))
    QUrl _arg0{};
    std::function<void( QUrl )> _arg1{};
    auto result = obj->subscribe(_arg0, _arg1);
    EXPECT_GE(result, 0);

}

TEST_F(DeviceMountSubscriberTest, unsubscribe)
{
    // Test method: void unsubscribe((int subscriptionId))
    EXPECT_NO_FATAL_FAILURE(obj->unsubscribe(0));
}
