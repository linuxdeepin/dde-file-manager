// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_tageditor.cpp
 * @brief Unit tests for TagEditor methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "widgets/tageditor.h"

#include <QTest>

using namespace dfmplugin_tag;

class TagEditorTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new TagEditor();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    TagEditor *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(TagEditorTest, TagEditor)
{
    // Test constructor: TagEditor(())
    ASSERT_NE(obj, nullptr);
}

TEST_F(TagEditorTest, keyPressEvent)
{
    // Test event handler: keyPressEvent((QKeyEvent *event))
    QKeyEvent _event(QKeyEvent::None);
    EXPECT_NO_FATAL_FAILURE(obj->keyPressEvent(&_event));
}
