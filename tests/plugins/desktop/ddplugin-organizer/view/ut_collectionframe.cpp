// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "view/collectionframe_p.h"
#include "view/collectionwidget_p.h"
#include "view/collectiontitlebar_p.h"
#include "mode/collectiondataprovider.h"

#include "stubext.h"
#include <gtest/gtest.h>

#include <QMouseEvent>
using namespace testing;
using namespace ddplugin_organizer;

#if 0
TEST(CollectionFrame, initUi)
{
    CollectionFrame frame;
    frame.initUi();
    EXPECT_NE(frame.d->mainLayout, nullptr);
    EXPECT_EQ(frame.contentsMargins(), QMargins(0, 0, 0, 0));
    delete frame.d->mainLayout;
}

TEST(CollectionFrame, updateStretchRect)
{
    CollectionFrame frame;
    frame.setGeometry(10, 5, 100, 90);
    frame.d->updateStretchRect();
    ASSERT_EQ(frame.d->stretchRects.size(), 8);
    EXPECT_EQ(frame.d->stretchRects.at(0), QRect(0, 0, 10, 10));
    EXPECT_EQ(frame.d->stretchRects.at(1), QRect(10, 0, 80, 10));
    EXPECT_EQ(frame.d->stretchRects.at(2), QRect(90, 0, 10, 10));
    EXPECT_EQ(frame.d->stretchRects.at(3), QRect(90, 10, 10, 70));
    EXPECT_EQ(frame.d->stretchRects.at(4), QRect(90, 80, 10, 10));
    EXPECT_EQ(frame.d->stretchRects.at(5), QRect(10, 80, 80, 10));
    EXPECT_EQ(frame.d->stretchRects.at(6), QRect(0, 80, 10, 10));
    EXPECT_EQ(frame.d->stretchRects.at(7), QRect(0, 10, 10, 70));
}

TEST(CollectionFrame, setWidget)
{
    CollectionFrame frame;
    CollectionWidget widget("1", nullptr);
    widget.d->titleBar->setGeometry(5, 5, 100, 40);

    frame.setWidget(&widget);
    EXPECT_EQ(frame.d->widget, &widget);
    EXPECT_EQ(frame.d->titleBarWidget, widget.d->titleBar);
    EXPECT_EQ(frame.d->titleBarRect, widget.d->titleBar->geometry());
}

TEST(CollectionFrame, updateFrameGeometry)
{
    CollectionFrame frame;
    const QRect base(10, 5, 100, 90);
    frame.setGeometry(base);
    frame.d->titleBarRect = base;

    {
        frame.d->responseArea = CollectionFramePrivate::LeftTopRect;
        frame.d->updateFrameGeometry();
        EXPECT_EQ(frame.geometry(), QRect(0, 0, 110, 95));
        EXPECT_EQ(frame.d->titleBarRect.width(), 110);
    }

    {
        frame.d->responseArea = CollectionFramePrivate::TopRect;
        frame.d->updateFrameGeometry();
        EXPECT_EQ(frame.geometry(), QRect(10, 0, 100, 95));
        EXPECT_EQ(frame.d->titleBarRect.width(), 100);
    }

    {
        frame.d->stretchBeforRect = base;
        frame.d->responseArea = CollectionFramePrivate::RightTopRect;
        frame.d->updateFrameGeometry();
        EXPECT_EQ(frame.geometry(), QRect(10, 0, 21, 95));
        EXPECT_EQ(frame.d->titleBarRect.width(), 21);
    }

    {
        frame.d->responseArea = CollectionFramePrivate::RightRect;
        frame.d->updateFrameGeometry();
        EXPECT_EQ(frame.geometry(), QRect(10, 5, 21, 90));
        EXPECT_EQ(frame.d->titleBarRect.width(), 21);
    }

    {
        frame.d->responseArea = CollectionFramePrivate::RightBottomRect;
        frame.d->updateFrameGeometry();
        EXPECT_EQ(frame.geometry(), QRect(10, 5, 21, 21));
        EXPECT_EQ(frame.d->titleBarRect.width(), 21);
    }

    {
        frame.d->responseArea = CollectionFramePrivate::BottomRect;
        frame.d->updateFrameGeometry();
        EXPECT_EQ(frame.geometry(), QRect(10, 5, 100, 21));
        EXPECT_EQ(frame.d->titleBarRect.width(), 100);
    }

    {
        frame.d->responseArea = CollectionFramePrivate::LeftBottomRect;
        frame.d->updateFrameGeometry();
        EXPECT_EQ(frame.geometry(), QRect(0, 5, 110, 21));
        EXPECT_EQ(frame.d->titleBarRect.width(), 110);
    }

    {
        frame.d->responseArea = CollectionFramePrivate::LeftRect;
        frame.d->updateFrameGeometry();
        EXPECT_EQ(frame.geometry(), QRect(0, 5, 110, 90));
        EXPECT_EQ(frame.d->titleBarRect.width(), 110);
    }

    {
        frame.setGeometry(base);
        frame.d->responseArea = CollectionFramePrivate::TitleBarRect;
        frame.d->updateFrameGeometry();
        EXPECT_EQ(frame.geometry(), base);
        EXPECT_EQ(frame.d->titleBarRect.width(), 110);
    }
}

TEST(CollectionFrame, mousePressEvent)
{
    CollectionFrame frame;
    QMouseEvent event(QEvent::Type::MouseButtonRelease, { 10, 10 }, Qt::MouseButton::LeftButton, Qt::MouseButton::LeftButton,
                      Qt::KeyboardModifier::AltModifier);

    frame.d->frameFeatures.setFlag(CollectionFrame::CollectionFrameStretchable);
    frame.d->stretchArea.push_back(CollectionFramePrivate::ResponseArea::UnKnowRect);
    frame.data->crect = QRect(1, 1, 2, 2);
    frame.mousePressEvent(&event);
    EXPECT_TRUE(event.m_accept);
    EXPECT_EQ(frame.d->stretchBeforRect, QRect(1, 1, 2, 2));
    EXPECT_EQ(frame.d->frameState, CollectionFramePrivate::StretchState);

    frame.d->frameFeatures.setFlag(CollectionFrame::CollectionFrameStretchable, false);
    frame.d->frameFeatures.setFlag(CollectionFrame::CollectionFrameMovable);
    frame.d->moveArea.push_back(CollectionFramePrivate::ResponseArea::UnKnowRect);
    frame.mousePressEvent(&event);
    EXPECT_EQ(frame.d->frameState, CollectionFramePrivate::MoveState);
    EXPECT_TRUE(event.m_accept);

    frame.d->frameFeatures.setFlag(CollectionFrame::CollectionFrameMovable, false);
    frame.mousePressEvent(&event);
    EXPECT_EQ(frame.d->frameState, CollectionFramePrivate::NormalShowState);
    EXPECT_TRUE(event.m_accept);
}

TEST(CollectionFrame, mouseReleaseEvent)
{
    stub_ext::StubExt stub;

    bool streachCall = false;
    stub.set_lamda(&CollectionFramePrivate::updateStretchRect, [&streachCall]() {
        __DBG_STUB_INVOKE__
        streachCall = true;
    });

    bool moveCall = false;
    stub.set_lamda(&CollectionFramePrivate::updateMoveRect, [&moveCall]() {
        __DBG_STUB_INVOKE__
        moveCall = true;
    });

    QMouseEvent event(QEvent::Type::MouseButtonRelease, { 10, 10 }, Qt::MouseButton::LeftButton, Qt::MouseButton::LeftButton,
                      Qt::KeyboardModifier::AltModifier);
    CollectionFrame frame;
    frame.d->frameFeatures.setFlag(CollectionFrame::CollectionFrameMovable);
    frame.d->frameFeatures.setFlag(CollectionFrame::CollectionFrameStretchable);

    frame.d->frameState = CollectionFramePrivate::StretchState;
    frame.mouseReleaseEvent(&event);
    EXPECT_EQ(frame.d->frameState, CollectionFramePrivate::NormalShowState);
    EXPECT_TRUE(streachCall);
    frame.d->frameState = CollectionFramePrivate::MoveState;
    frame.mouseReleaseEvent(&event);
    EXPECT_EQ(frame.d->frameState, CollectionFramePrivate::NormalShowState);
    EXPECT_TRUE(moveCall);
    EXPECT_TRUE(event.m_accept);
}

TEST(CollectionFrame, mouseMoveEvent)
{
    stub_ext::StubExt stub;
    bool update = false;
    stub.set_lamda(&CollectionFramePrivate::updateFrameGeometry, [&update]() {
        __DBG_STUB_INVOKE__
        update = true;
    });
    QMouseEvent event(QEvent::Type::MouseButtonRelease, { 10, 10 }, Qt::MouseButton::LeftButton, Qt::MouseButton::LeftButton,
                      Qt::KeyboardModifier::AltModifier);
    CollectionFrame frame;

    event.buttons().setFlag((Qt::LeftButton));
    frame.d->frameFeatures.setFlag(CollectionFrame::CollectionFrameStretchable);
    frame.d->frameState = CollectionFramePrivate::StretchState;

    bool connect = false;
    QObject::connect(&frame, &CollectionFrame::geometryChanged, &frame, [&connect]() {
        connect = true;
    });

    frame.mouseMoveEvent(&event);
    EXPECT_TRUE(connect);
    EXPECT_TRUE(update);

    frame.d->frameFeatures.setFlag(CollectionFrame::CollectionFrameStretchable, false);
    frame.d->frameFeatures.setFlag(CollectionFrame::CollectionFrameMovable);

    frame.mouseMoveEvent(&event);
    EXPECT_TRUE(connect);

    connect = false;
    event.buttons().setFlag(Qt::LeftButton, false);
    frame.mouseMoveEvent(&event);
    EXPECT_FALSE(connect);
    EXPECT_TRUE(event.m_accept);
}

TEST(CollectionFrame, resizeEvent)
{
    stub_ext::StubExt stub;

    bool streachCall = false;
    stub.set_lamda(&CollectionFramePrivate::updateStretchRect, [&streachCall]() {
        __DBG_STUB_INVOKE__
        streachCall = true;
    });

    bool moveCall = false;
    stub.set_lamda(&CollectionFramePrivate::updateMoveRect, [&moveCall]() {
        __DBG_STUB_INVOKE__
        moveCall = true;
    });
    QResizeEvent event(QSize(1, 1), QSize(2, 2));
    CollectionFrame frame;

    frame.d->frameFeatures.setFlag(CollectionFrame::CollectionFrameStretchable);
    frame.resizeEvent(&event);
    EXPECT_TRUE(streachCall);
    frame.d->frameFeatures.setFlag(CollectionFrame::CollectionFrameStretchable, false);
    frame.d->frameFeatures.setFlag(CollectionFrame::CollectionFrameMovable);
    frame.resizeEvent(&event);
    EXPECT_TRUE(moveCall);
}
TEST(CollectionFrame, Event)
{
    QPaintEvent event1(QRect(1, 1, 2, 2));
    CollectionFrame frame;

    frame.paintEvent(&event1);
    EXPECT_TRUE(event1.m_accept);

    QFocusEvent event2(QEvent::FocusIn);
    frame.focusOutEvent(&event2);
    EXPECT_EQ(frame.cursor(), Qt::ArrowCursor);

    frame.initUi();
    EXPECT_TRUE(frame.testAttribute(Qt::WA_TranslucentBackground));
    EXPECT_FALSE(frame.autoFillBackground());
    EXPECT_EQ(frame.contentsMargins(), QMargins(0, 0, 0, 0));
    EXPECT_EQ(frame.d->mainLayout->contentsMargins(), QMargins(0, 0, 0, 0));

    delete frame.d->mainLayout;
}

class UT_CollectionFramePrivate : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        frame_q = new CollectionFrame;
        frame = frame_q->d.get();
    }
    virtual void TearDown() override
    {
        delete frame_q;
        stub.clear();
    }
    CollectionFrame *frame_q = nullptr;
    CollectionFramePrivate *frame = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(UT_CollectionFramePrivate, updateMoveRect)
{
    QWidget widget;
    widget.data->crect = QRect(1, 1, 2, 2);
    frame->titleBarWidget = &widget;
    frame->updateMoveRect();
    EXPECT_EQ(frame->titleBarRect, QRect(1, 1, 2, 2));
}

TEST_F(UT_CollectionFramePrivate, getCurrentResponseArea)
{
    frame->stretchRects.push_back(QRect(2, 2, 3, 3));
    auto res = frame->getCurrentResponseArea(QPoint(2, 2));
    EXPECT_EQ(res, CollectionFramePrivate::ResponseArea::LeftTopRect);
    frame->titleBarRect = QRect(0, 0, 1, 1);
    res = frame->getCurrentResponseArea(QPoint(0.5, 0.5));
    EXPECT_EQ(res, CollectionFramePrivate::ResponseArea::TitleBarRect);
}

TEST_F(UT_CollectionFramePrivate, updateCursorState)
{

    frame->frameFeatures.setFlag(CollectionFrame::CollectionFrameMovable);
    frame->frameFeatures.setFlag(CollectionFrame::CollectionFrameStretchable);
    CollectionFramePrivate::ResponseArea stretchPlace;

    stretchPlace = CollectionFramePrivate::ResponseArea::RightBottomRect;
    frame->updateCursorState(stretchPlace);
    EXPECT_EQ(frame->q->cursor(), Qt::SizeFDiagCursor);

    stretchPlace = CollectionFramePrivate::ResponseArea::BottomRect;
    frame->updateCursorState(stretchPlace);
    EXPECT_EQ(frame->q->cursor(), Qt::SizeVerCursor);

    stretchPlace = CollectionFramePrivate::ResponseArea::LeftBottomRect;
    frame->updateCursorState(stretchPlace);
    EXPECT_EQ(frame->q->cursor(), Qt::SizeBDiagCursor);

    stretchPlace = CollectionFramePrivate::ResponseArea::LeftRect;
    frame->updateCursorState(stretchPlace);
    EXPECT_EQ(frame->q->cursor(), Qt::SizeHorCursor);

    stretchPlace = CollectionFramePrivate::ResponseArea::TitleBarRect;
    frame->updateCursorState(stretchPlace);
    EXPECT_EQ(frame->q->cursor(), Qt::SizeAllCursor);

    frame->frameFeatures.setFlag(CollectionFrame::CollectionFrameMovable, false);
    frame->updateCursorState(stretchPlace);
    EXPECT_EQ(frame->q->cursor(), Qt::ArrowCursor);
}
#endif
