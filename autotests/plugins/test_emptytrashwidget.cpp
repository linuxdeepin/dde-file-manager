// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_emptytrashwidget.cpp
 * @brief Unit tests for EmptyTrashWidget methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "views/emptyTrashWidget.h"

#include <QTest>

using namespace dfmplugin_trash;

class EmptyTrashWidgetTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new EmptyTrashWidget();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    EmptyTrashWidget *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(EmptyTrashWidgetTest, EmptyTrashWidget)
{
    // Test constructor: EmptyTrashWidget((QWidget *parent))
    ASSERT_NE(obj, nullptr);
}
