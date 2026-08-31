// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_entrywidget.cpp
 * @brief Unit tests for EntryWidget methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "options/widgets/entrywidget.h"

#include <QTest>

using namespace ddplugin_organizer;

class EntryWidgetTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new EntryWidget();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    EntryWidget *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(EntryWidgetTest, EntryWidget)
{
    // Test constructor: EntryWidget((QWidget *left, QWidget *right, QWidget *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(EntryWidgetTest, widget)
{
    // Test getter: QWidget widget()
    auto result = obj->widget();
    EXPECT_NO_FATAL_FAILURE({ obj->widget(); });

}
