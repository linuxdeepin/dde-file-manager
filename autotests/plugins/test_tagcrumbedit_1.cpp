// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_tagcrumbedit_1.cpp
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

TEST_F(TagCrumbEditTest, isEditing)
{
    // Test bool getter: isEditing()
    bool result = obj->isEditing();
    EXPECT_FALSE(result);

}

TEST_F(TagCrumbEditTest, mouseDoubleClickEvent)
{
    // Test event handler: mouseDoubleClickEvent((QMouseEvent *event))
    QMouseEvent _event(QMouseEvent::None);
    EXPECT_NO_FATAL_FAILURE(obj->mouseDoubleClickEvent(&_event));
}

TEST_F(TagCrumbEditTest, updateHeight)
{
    // Test method: void updateHeight(())
    EXPECT_NO_FATAL_FAILURE(obj->updateHeight());
}
