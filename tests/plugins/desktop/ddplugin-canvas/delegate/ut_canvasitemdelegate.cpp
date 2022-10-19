/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangyu<zhangyub@uniontech.com>
 *
 * Maintainer: zhangyu<zhangyub@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

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

