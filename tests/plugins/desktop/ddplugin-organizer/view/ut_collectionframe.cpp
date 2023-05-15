// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "view/collectionframe_p.h"
#include "view/collectionwidget_p.h"
#include "view/collectiontitlebar_p.h"
#include "mode/collectiondataprovider.h"

#include "stubext.h"

#include <gtest/gtest.h>

using namespace testing;
using namespace ddplugin_organizer;

TEST(CollectionFrame, initUi)
{
    CollectionFrame frame;
    frame.initUi();
    EXPECT_NE(frame.d->mainLayout, nullptr);
    EXPECT_EQ(frame.contentsMargins(), QMargins(0,0,0,0));
}

TEST(CollectionFrame, updateStretchRect)
{
    CollectionFrame frame;
    frame.setGeometry(10, 5, 100, 90);
    frame.d->updateStretchRect();
    ASSERT_EQ(frame.d->stretchRects.size(), 8);
    EXPECT_EQ(frame.d->stretchRects.at(0), QRect(0,0,10,10));
    EXPECT_EQ(frame.d->stretchRects.at(1), QRect(10,0,80,10));
    EXPECT_EQ(frame.d->stretchRects.at(2), QRect(90,0,10,10));
    EXPECT_EQ(frame.d->stretchRects.at(3), QRect(90,10,10,70));
    EXPECT_EQ(frame.d->stretchRects.at(4), QRect(90,80,10,10));
    EXPECT_EQ(frame.d->stretchRects.at(5), QRect(10,80,80,10));
    EXPECT_EQ(frame.d->stretchRects.at(6), QRect(0,80,10,10));
    EXPECT_EQ(frame.d->stretchRects.at(7), QRect(0,10,10,70));
}

TEST(CollectionFrame, setWidget)
{
    CollectionFrame frame;
    CollectionWidget widget("1", nullptr);
    widget.d->titleBar->setGeometry(5,5,100,40);

    frame.setWidget(&widget);
    EXPECT_EQ(frame.d->widget, &widget);
    EXPECT_EQ(frame.d->titleBarWidget, widget.d->titleBar);
    EXPECT_EQ(frame.d->titleBarRect, widget.d->titleBar->geometry());
    EXPECT_EQ(frame.d->minHeight, 24);
}

TEST(CollectionFrame, updateFrameGeometry)
{
    CollectionFrame frame;
    const QRect base(10, 5, 100, 90);
    frame.setGeometry(base);
    frame.d->titleBarRect = base;

    {
        frame.d->stretchBeforRect = base;
        frame.d->responseArea = CollectionFramePrivate::LeftTopRect;
        frame.d->updateFrameGeometry();
        EXPECT_EQ(frame.geometry(), QRect(0,0,110,95));
        EXPECT_EQ(frame.d->titleBarRect.width(), 110);
    }

    {
        frame.d->stretchBeforRect = base;
        frame.d->responseArea = CollectionFramePrivate::TopRect;
        frame.d->updateFrameGeometry();
        EXPECT_EQ(frame.geometry(), QRect(10,0,100,95));
        EXPECT_EQ(frame.d->titleBarRect.width(), 100);
    }

    {
        frame.d->stretchBeforRect = base;
        frame.d->responseArea = CollectionFramePrivate::RightTopRect;
        frame.d->updateFrameGeometry();
        EXPECT_EQ(frame.geometry(), QRect(10,0,21,95));
        EXPECT_EQ(frame.d->titleBarRect.width(), 21);
    }

    {
        frame.d->stretchBeforRect = base;
        frame.d->responseArea = CollectionFramePrivate::RightRect;
        frame.d->updateFrameGeometry();
        EXPECT_EQ(frame.geometry(), QRect(10,5,21,90));
        EXPECT_EQ(frame.d->titleBarRect.width(), 21);
    }

    {
        frame.d->stretchBeforRect = base;
        frame.d->responseArea = CollectionFramePrivate::RightBottomRect;
        frame.d->updateFrameGeometry();
        EXPECT_EQ(frame.geometry(), QRect(10,5,21,21));
        EXPECT_EQ(frame.d->titleBarRect.width(), 21);
    }

    {
        frame.d->stretchBeforRect = base;
        frame.d->responseArea = CollectionFramePrivate::BottomRect;
        frame.d->updateFrameGeometry();
        EXPECT_EQ(frame.geometry(), QRect(10,5,100,21));
        EXPECT_EQ(frame.d->titleBarRect.width(), 100);
    }

    {
        frame.d->stretchBeforRect = base;
        frame.d->responseArea = CollectionFramePrivate::LeftBottomRect;
        frame.d->updateFrameGeometry();
        EXPECT_EQ(frame.geometry(), QRect(0,5,110,21));
        EXPECT_EQ(frame.d->titleBarRect.width(), 110);
    }

    {
        frame.d->stretchBeforRect = base;
        frame.d->responseArea = CollectionFramePrivate::LeftRect;
        frame.d->updateFrameGeometry();
        EXPECT_EQ(frame.geometry(), QRect(0,5,110,90));
        EXPECT_EQ(frame.d->titleBarRect.width(), 110);
    }

    {
        frame.d->stretchBeforRect = base;
        frame.setGeometry(base);
        frame.d->responseArea = CollectionFramePrivate::TitleBarRect;
        frame.d->updateFrameGeometry();
        EXPECT_EQ(frame.geometry(), base);
        EXPECT_EQ(frame.d->titleBarRect.width(), 110);
    }
}
