// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_viewoptionsbutton.cpp
 * @brief Unit tests for ViewOptionsButton methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "views/viewoptionsbutton.h"

#include <QTest>

using namespace dfmplugin_titlebar;

class ViewOptionsButtonTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new ViewOptionsButton();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    ViewOptionsButton *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(ViewOptionsButtonTest, event)
{
    // Test method: bool event((QEvent *event))
    auto result = obj->event(nullptr);
    EXPECT_FALSE(result);

}
