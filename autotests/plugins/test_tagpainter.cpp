// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_tagpainter.cpp
 * @brief Unit tests for TagPainter methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "utils/tagpainter.h"

#include <QTest>

using namespace dfmplugin_tag;

class TagPainterTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new TagPainter();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    TagPainter *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(TagPainterTest, drawObject)
{
    // Test method: void drawObject((QPainter *painter, const QRectF &rect, QTextDocument *doc, int posInDocument, const QTextFormat &format))
    QRectF _arg1{};
    QTextFormat _arg4{};
    EXPECT_NO_FATAL_FAILURE(obj->drawObject(nullptr, _arg1, nullptr, 0, _arg4));
}

TEST_F(TagPainterTest, intrinsicSize)
{
    // Test method: QSizeF intrinsicSize((QTextDocument *doc, int posInDocument, const QTextFormat &format))
    QTextFormat _arg2{};
    auto result = obj->intrinsicSize(nullptr, 0, _arg2);
    EXPECT_TRUE(result.isEmpty());

}
