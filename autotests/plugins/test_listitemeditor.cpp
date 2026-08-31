// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_listitemeditor.cpp
 * @brief Unit tests for ListItemEditor methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "views/listitemeditor.h"

#include <QTest>

using namespace dfmplugin_workspace;

class ListItemEditorTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new ListItemEditor();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    ListItemEditor *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(ListItemEditorTest, event)
{
    // Test method: bool event((QEvent *ee))
    auto result = obj->event(nullptr);
    EXPECT_FALSE(result);

}
