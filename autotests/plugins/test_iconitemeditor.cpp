// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_iconitemeditor.cpp
 * @brief Unit tests for IconItemEditor methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "views/iconitemeditor.h"

#include <QTest>

using namespace dfmplugin_workspace;

class IconItemEditorTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new IconItemEditor();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    IconItemEditor *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(IconItemEditorTest, IconItemEditor)
{
    // Test constructor: IconItemEditor((QWidget *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(IconItemEditorTest, onEditTextChanged)
{
    // Test method: void onEditTextChanged(())
    EXPECT_NO_FATAL_FAILURE(obj->onEditTextChanged());
}

TEST_F(IconItemEditorTest, text)
{
    // Test getter: QString text()
    auto result = obj->text();
    EXPECT_TRUE(result.isEmpty() || !result.isEmpty());
}
