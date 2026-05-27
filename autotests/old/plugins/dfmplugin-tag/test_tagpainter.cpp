// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "plugins/common/dfmplugin-tag/utils/tagpainter.h"

#include <gtest/gtest.h>

#include <QPainter>
#include <QPixmap>
#include <QTextFormat>

using namespace dfmplugin_tag;

class UT_TagPainter : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        painter = new TagPainter();
    }

    virtual void TearDown() override
    {
        delete painter;
        painter = nullptr;
        stub.clear();
    }

public:
    stub_ext::StubExt stub;
    TagPainter *painter = nullptr;
};

TEST_F(UT_TagPainter, constructor)
{
    // Test constructor
    EXPECT_NE(painter, nullptr);
}

TEST_F(UT_TagPainter, drawObject)
{
    // Test drawing object
    QPixmap pixmap(100, 100);
    QPainter qpainter(&pixmap);
    QRectF rect(0, 0, 100, 100);
    QTextDocument doc;
    int posInDocument = 0;
    QTextFormat format;

    // drawObject is virtual, test it doesn't crash
    EXPECT_NO_THROW(painter->drawObject(&qpainter, rect, &doc, posInDocument, format));
}

TEST_F(UT_TagPainter, intrinsicSize)
{
    // Test intrinsic size calculation
    QTextDocument doc;
    int posInDocument = 0;
    QTextFormat format;

    QSizeF size = painter->intrinsicSize(&doc, posInDocument, format);

    // Size should be non-negative
    EXPECT_GE(size.width(), 0);
    EXPECT_GE(size.height(), 0);
}
