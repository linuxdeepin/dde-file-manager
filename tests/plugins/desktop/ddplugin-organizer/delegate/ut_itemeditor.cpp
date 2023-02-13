// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UT_ITEMEDITOR_CPP
#define UT_ITEMEDITOR_CPP

#include "delegate/itemeditor.h"
#include "view/collectionview_p.h"

#include "stubext.h"

#include <QLayout>

#include <DStyle>

#include <gtest/gtest.h>

DWIDGET_USE_NAMESPACE
using namespace ddplugin_organizer;

TEST(ItemEditor, construct)
{
    ItemEditor ie;
    EXPECT_EQ(ie.maxTextLength, INT_MAX);
    EXPECT_EQ(ie.maxHeight, -1);
    EXPECT_NE(ie.textEditor, nullptr);
    EXPECT_EQ(ie.opacityEffect, nullptr);
    EXPECT_EQ(ie.tooltip, nullptr);

    EXPECT_EQ(ie.focusProxy(), ie.textEditor);
}

TEST(ItemEditor, editor)
{
    ItemEditor ie;
    EXPECT_EQ(ie.textEditor, ie.editor());
}

TEST(ItemEditor, setMaxHeight)
{
    ItemEditor ie;
    int h = 1000;
    ie.setMaxHeight(h);
    EXPECT_EQ(ie.maxHeight, h);
}

TEST(ItemEditor, updateGeometry)
{
    ItemEditor ie;
    ie.resize(100, 300);
    ie.itemSizeHint = QSize(90, 200);
    ie.layout()->setContentsMargins(0, 50, 0, 0);
    auto it = ie.textEditor;

    stub_ext::StubExt stub;
    stub.set_lamda(&QTextDocument::size, [](){
        return QSize(90, 300);
    });

    ASSERT_FALSE(it->isReadOnly());
    ie.maxHeight = 30;
    ie.updateGeometry();

    EXPECT_EQ(it->width(), 100);
    //EXPECT_EQ(it->document()->textWidth(), 90); why is it 100?
    EXPECT_EQ(it->height(), ie.fontMetrics().height() * 3 + 4);

    int minHeight = ie.fontMetrics().height() * 1 + 4;
    ie.maxHeight = minHeight + 25;

    ie.updateGeometry();
    EXPECT_EQ(it->height(), minHeight);

    ie.maxHeight = 300 + 50 + 1;

    ie.updateGeometry();
    EXPECT_EQ(it->height(), 300);

    ie.maxHeight = 300 + 50 - 1;

    ie.updateGeometry();
    EXPECT_EQ(it->height(), ie.maxHeight - 50);
}

TEST(RenameEdit, adjustStyle)
{
    RenameEdit re;
    ASSERT_NE(re.document(), nullptr);
    EXPECT_EQ(re.document()->documentMargin(), 2);
    const int frameRadius = DStyle::pixelMetric(re.style(), DStyle::PM_FrameRadius, nullptr, &re);
    EXPECT_EQ(frameRadius, 0);
}


#endif // UT_ITEMEDITOR_CPP
