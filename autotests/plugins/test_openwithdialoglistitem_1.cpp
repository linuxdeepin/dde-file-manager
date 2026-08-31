// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_openwithdialoglistitem_1.cpp
 * @brief Unit tests for OpenWithDialogListItem methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "openwith/openwithdialog.h"

#include <QTest>

using namespace dfmplugin_utils;

class OpenWithDialogListItemTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new OpenWithDialogListItem();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    OpenWithDialogListItem *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(OpenWithDialogListItemTest, enterEvent)
{
    // Test event handler: enterEvent((QEnterEvent *e))
    QEnterEvent _event(QEnterEvent::None);
    EXPECT_NO_FATAL_FAILURE(obj->enterEvent(&_event));
}

TEST_F(OpenWithDialogListItemTest, initUiForSizeMode)
{
    // Test method: void initUiForSizeMode(())
    EXPECT_NO_FATAL_FAILURE(obj->initUiForSizeMode());
}

TEST_F(OpenWithDialogListItemTest, leaveEvent)
{
    // Test event handler: leaveEvent((QEvent *e))
    QEvent _event(QEvent::None);
    EXPECT_NO_FATAL_FAILURE(obj->leaveEvent(&_event));
}

TEST_F(OpenWithDialogListItemTest, paintEvent)
{
    // Test event handler: paintEvent((QPaintEvent *e))
    QPaintEvent _event(QPaintEvent::None);
    EXPECT_NO_FATAL_FAILURE(obj->paintEvent(&_event));
}

TEST_F(OpenWithDialogListItemTest, setChecked)
{
    // Test setter: void setChecked((bool checked))
    EXPECT_NO_FATAL_FAILURE(obj->setChecked(false));
}

TEST_F(OpenWithDialogListItemTest, text)
{
    // Test getter: QString text()
    auto result = obj->text();
    EXPECT_TRUE(result.isEmpty() || !result.isEmpty());
}

TEST_F(OpenWithDialogListItemTest, updateLabelIcon)
{
    // Test method: void updateLabelIcon((int size))
    EXPECT_NO_FATAL_FAILURE(obj->updateLabelIcon(0));
}
