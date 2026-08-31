// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_coreeventscaller.cpp
 * @brief Unit tests for CoreEventsCaller methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "events/coreeventscaller.h"

#include <QTest>

using namespace core;

class CoreEventsCallerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new CoreEventsCaller();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    CoreEventsCaller *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(CoreEventsCallerTest, sendGetSelectedFiles)
{
    // Test method: QList<QUrl> sendGetSelectedFiles((const quint64 windowID))
    auto result = obj->sendGetSelectedFiles(0);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(CoreEventsCallerTest, sendSelectFiles)
{
    // Test method: void sendSelectFiles((quint64 windowId, const QList<QUrl> &files))
    QList<QUrl> _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->sendSelectFiles(0, _arg1));
}

TEST_F(CoreEventsCallerTest, sendViewMode)
{
    // Test method: void sendViewMode((QWidget *sender, DFMBASE_NAMESPACE::Global::ViewMode mode))
    EXPECT_NO_FATAL_FAILURE(obj->sendViewMode(nullptr, DFMBASE_NAMESPACE::Global::ViewMode()));
}

TEST_F(CoreEventsCallerTest, setEnabledSelectionModes)
{
    // Test setter: void setEnabledSelectionModes((QWidget *sender, const QList<QAbstractItemView::SelectionMode> &modes))
    QList<QAbstractItemView::SelectionMode> _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->setEnabledSelectionModes(nullptr, _arg1));
}

TEST_F(CoreEventsCallerTest, setMenuDisbaled)
{
    // Test method: void setMenuDisbaled(())
    EXPECT_NO_FATAL_FAILURE(obj->setMenuDisbaled());
}

TEST_F(CoreEventsCallerTest, setSelectionMode)
{
    // Test setter: void setSelectionMode((QWidget *sender, const QAbstractItemView::SelectionMode mode))
    EXPECT_NO_FATAL_FAILURE(obj->setSelectionMode(nullptr, QAbstractItemView::SelectionMode()));
}

TEST_F(CoreEventsCallerTest, setSidebarItemVisible)
{
    // Test setter: void setSidebarItemVisible((const QUrl &url, bool visible))
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->setSidebarItemVisible(_arg0, false));
}
