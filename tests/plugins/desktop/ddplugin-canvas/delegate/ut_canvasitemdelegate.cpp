// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "delegate/canvasitemdelegate_p.h"
#include "view/canvasview_p.h"

#include "stubext.h"

#include <gtest/gtest.h>

DDP_CANVAS_USE_NAMESPACE

TEST(CanvasItemDelegate, iconLevelRange)
{
    CanvasView view;
    CanvasItemDelegate obj(&view);

    EXPECT_EQ(obj.minimumIconLevel(), 0);
    EXPECT_EQ(obj.maximumIconLevel(), 4);
}

TEST(CanvasItemDelegate, updateItemSizeHint)
{
    CanvasView view;
    CanvasItemDelegate obj(&view);

    stub_ext::StubExt stub;
    int height = 20;
    stub.set_lamda(&QFontMetrics::height, [&height](){
        return height;
    });

    QSize icon = QSize(50, 50);
    stub.set_lamda(&CanvasView::iconSize, [&icon](){
        return icon;
    });

    obj.updateItemSizeHint();
    EXPECT_EQ(obj.d->textLineHeight, height);
    EXPECT_EQ(obj.d->itemSizeHint.width(), icon.width() * 17 / 10);
    EXPECT_EQ(obj.d->itemSizeHint.height(), icon.height() + 10 + 2 * height);

    height = 30;
    icon = QSize(60, 60);
    obj.updateItemSizeHint();
    EXPECT_EQ(obj.d->textLineHeight, height);
    EXPECT_EQ(obj.d->itemSizeHint.width(), icon.width() * 17 / 10);
    EXPECT_EQ(obj.d->itemSizeHint.height(), icon.height() + 10 + 2 * height);
}

TEST(CanvasItemDelegatePrivate, needExpend)
{
    CanvasView view;
    CanvasItemDelegate obj(&view);

    stub_ext::StubExt stub;
    stub.set_lamda(&CanvasItemDelegate::textPaintRect, [](){
        return QRect(100, 100, 100, 200);
    });

    QRect out;
    QRect in(10, 10, 100, 100);
    const QRect need(10, 10, 100, 200);
    EXPECT_TRUE(obj.d->needExpend({}, {}, in, &out));
    EXPECT_EQ(out, need);

    in = QRect(10, 10, 100, 300);
    EXPECT_FALSE(obj.d->needExpend({}, {}, in, &out));
    EXPECT_EQ(out, need);
}

