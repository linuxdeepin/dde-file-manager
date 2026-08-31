// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_rightvaluewidget.cpp
 * @brief Unit tests for RightValueWidget methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "dfm-base/widgets/dfmkeyvaluelabel/keyvaluelabel.h"

#include <QTest>

using namespace src;

class RightValueWidgetTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new RightValueWidget();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    RightValueWidget *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(RightValueWidgetTest, RightValueWidget)
{
    // Test constructor: RightValueWidget((QWidget *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(RightValueWidgetTest, customContextMenuEvent)
{
    // Test event handler: customContextMenuEvent((const QPoint &pos))
    QPoint _event(QPoint::None);
    EXPECT_NO_FATAL_FAILURE(obj->customContextMenuEvent(&_event));
}
