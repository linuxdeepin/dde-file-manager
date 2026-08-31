// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_propertyeventreceiver_1.cpp
 * @brief Unit tests for PropertyEventReceiver methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "events/propertyeventreceiver.h"

#include <QTest>

using namespace dfmplugin_propertydialog;

class PropertyEventReceiverTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new PropertyEventReceiver();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    PropertyEventReceiver *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(PropertyEventReceiverTest, PropertyEventReceiver)
{
    // Test constructor: PropertyEventReceiver((QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(PropertyEventReceiverTest, bindEvents)
{
    // Test method: void bindEvents(())
    EXPECT_NO_FATAL_FAILURE(obj->bindEvents());
}

TEST_F(PropertyEventReceiverTest, handleBasicFiledFilterAdd)
{
    // Test method: bool handleBasicFiledFilterAdd((const QString &scheme, const QStringList &enums))
    QString _arg0{};
    QStringList _arg1{};
    auto result = obj->handleBasicFiledFilterAdd(_arg0, _arg1);
    EXPECT_FALSE(result);

}

TEST_F(PropertyEventReceiverTest, handleViewExtensionRegisterWithUpdate)
{
    // Test method: bool handleViewExtensionRegisterWithUpdate((CustomViewExtensionView creator,
                                                                  ViewExtensionUpdateFunc updater,
                                                                  const QString &name, int index))
    QString _arg2{};
    auto result = obj->handleViewExtensionRegisterWithUpdate(CustomViewExtensionView(), ViewExtensionUpdateFunc(), _arg2, 0);
    EXPECT_FALSE(result);

}

TEST_F(PropertyEventReceiverTest, instance)
{
    // Test getter: PropertyEventReceiver instance()
    auto result = obj->instance();
    EXPECT_NO_FATAL_FAILURE({ obj->instance(); });

}
