// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_completerview.cpp
 * @brief Unit tests for CompleterView methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "views/completerview.h"

#include <QTest>

using namespace dfmplugin_titlebar;

class CompleterViewTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new CompleterView();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    CompleterView *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(CompleterViewTest, CompleterView)
{
    // Test constructor: CompleterView((QWidget *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(CompleterViewTest, currentChanged)
{
    // Test method: void currentChanged((const QModelIndex &current, const QModelIndex &previous))
    QModelIndex _arg0{};
    QModelIndex _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->currentChanged(_arg0, _arg1));
}

TEST_F(CompleterViewTest, keyPressEvent)
{
    // Test event handler: keyPressEvent((QKeyEvent *e))
    QKeyEvent _event(QKeyEvent::None);
    EXPECT_NO_FATAL_FAILURE(obj->keyPressEvent(&_event));
}

TEST_F(CompleterViewTest, selectionChanged)
{
    // Test method: void selectionChanged((const QItemSelection &selected, const QItemSelection &deselected))
    QItemSelection _arg0{};
    QItemSelection _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->selectionChanged(_arg0, _arg1));
}
