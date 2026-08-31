// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_tagcrumbedit.cpp
 * @brief Unit tests for TagCrumbEdit methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "widgets/tagcrumbedit.h"

#include <QTest>

using namespace dfmplugin_tag;

class TagCrumbEditTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new TagCrumbEdit();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    TagCrumbEdit *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(TagCrumbEditTest, TagCrumbEdit)
{
    // Test constructor: TagCrumbEdit((QWidget *parent))
    ASSERT_NE(obj, nullptr);
}
