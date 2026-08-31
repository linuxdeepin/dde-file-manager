// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_propertyeventreceiver.cpp
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

TEST_F(PropertyEventReceiverTest, handleBasicViewExtensionRegister)
{
    // Test method: bool handleBasicViewExtensionRegister((BasicViewFieldFunc func, const QString &scheme))
    QString _arg1{};
    auto result = obj->handleBasicViewExtensionRegister(BasicViewFieldFunc(), _arg1);
    EXPECT_FALSE(result);

}

TEST_F(PropertyEventReceiverTest, handleCustomViewRegister)
{
    // Test method: bool handleCustomViewRegister((CustomViewExtensionView view, const QString &scheme))
    QString _arg1{};
    auto result = obj->handleCustomViewRegister(CustomViewExtensionView(), _arg1);
    EXPECT_FALSE(result);

}

TEST_F(PropertyEventReceiverTest, handleShowPropertyDialog)
{
    // Test method: void handleShowPropertyDialog((const QList<QUrl> &urls, const QVariantHash &option))
    QList<QUrl> _arg0{};
    QVariantHash _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->handleShowPropertyDialog(_arg0, _arg1));
}

TEST_F(PropertyEventReceiverTest, handleViewExtensionRegister)
{
    // Test method: bool handleViewExtensionRegister((CustomViewExtensionView view, const QString &name, int index))
    QString _arg1{};
    auto result = obj->handleViewExtensionRegister(CustomViewExtensionView(), _arg1, 0);
    EXPECT_FALSE(result);

}
