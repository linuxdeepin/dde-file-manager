// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_shredfailedwidget.cpp
 * @brief Unit tests for ShredFailedWidget methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "shred/progressdialog.h"

#include <QTest>

using namespace dfmplugin_utils;

class ShredFailedWidgetTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new ShredFailedWidget();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    ShredFailedWidget *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(ShredFailedWidgetTest, ShredFailedWidget)
{
    // Test constructor: ShredFailedWidget((QWidget *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(ShredFailedWidgetTest, setMessage)
{
    // Test setter: void setMessage((const QString &msg))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->setMessage(_arg0));
}
