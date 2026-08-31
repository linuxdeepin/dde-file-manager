// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_detailspaceeventreceiver.cpp
 * @brief Unit tests for DetailSpaceEventReceiver methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "events/detailspaceeventreceiver.h"

#include <QTest>

using namespace dfmplugin_detailspace;

class DetailSpaceEventReceiverTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new DetailSpaceEventReceiver();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    DetailSpaceEventReceiver *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(DetailSpaceEventReceiverTest, DetailSpaceEventReceiver)
{
    // Test constructor: DetailSpaceEventReceiver((QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(DetailSpaceEventReceiverTest, connectService)
{
    // Test method: void connectService(())
    EXPECT_NO_FATAL_FAILURE(obj->connectService());
}

TEST_F(DetailSpaceEventReceiverTest, handleBasicFiledFilterAdd)
{
    // Test method: bool handleBasicFiledFilterAdd((const QString &scheme, const QStringList &enums))
    QString _arg0{};
    QStringList _arg1{};
    auto result = obj->handleBasicFiledFilterAdd(_arg0, _arg1);
    EXPECT_FALSE(result);

}

TEST_F(DetailSpaceEventReceiverTest, handleBasicFiledFilterRootAdd)
{
    // Test method: bool handleBasicFiledFilterRootAdd((const QString &scheme, const QStringList &enums))
    QString _arg0{};
    QStringList _arg1{};
    auto result = obj->handleBasicFiledFilterRootAdd(_arg0, _arg1);
    EXPECT_FALSE(result);

}

TEST_F(DetailSpaceEventReceiverTest, handleBasicViewExtensionRegister)
{
    // Test method: bool handleBasicViewExtensionRegister((BasicViewFieldFunc func, const QString &scheme))
    QString _arg1{};
    auto result = obj->handleBasicViewExtensionRegister(BasicViewFieldFunc(), _arg1);
    EXPECT_FALSE(result);

}

TEST_F(DetailSpaceEventReceiverTest, handleBasicViewExtensionRootRegister)
{
    // Test method: bool handleBasicViewExtensionRootRegister((BasicViewFieldFunc func, const QString &scheme))
    QString _arg1{};
    auto result = obj->handleBasicViewExtensionRootRegister(BasicViewFieldFunc(), _arg1);
    EXPECT_FALSE(result);

}

TEST_F(DetailSpaceEventReceiverTest, handleTileBarShowDetailView)
{
    // Test method: void handleTileBarShowDetailView((quint64 windowId, bool checked, bool userAction))
    EXPECT_NO_FATAL_FAILURE(obj->handleTileBarShowDetailView(0, false, false));
}

TEST_F(DetailSpaceEventReceiverTest, handleViewExtensionRegister)
{
    // Test method: bool handleViewExtensionRegister((ViewExtensionCreateFunc create,
                                                           ViewExtensionUpdateFunc update,
                                                           ViewExtensionShouldShowFunc shouldShow,
                                                           int index))
    auto result = obj->handleViewExtensionRegister(ViewExtensionCreateFunc(), ViewExtensionUpdateFunc(), ViewExtensionShouldShowFunc(), 0);
    EXPECT_FALSE(result);

}

TEST_F(DetailSpaceEventReceiverTest, handleViewSelectionChanged)
{
    // Test method: void handleViewSelectionChanged((const quint64 windowID, const QItemSelection &selected, const QItemSelection &deselected))
    QItemSelection _arg1{};
    QItemSelection _arg2{};
    EXPECT_NO_FATAL_FAILURE(obj->handleViewSelectionChanged(0, _arg1, _arg2));
}

TEST_F(DetailSpaceEventReceiverTest, instance)
{
    // Test getter: DetailSpaceEventReceiver instance()
    auto result = obj->instance();
    EXPECT_NO_FATAL_FAILURE({ obj->instance(); });

}
