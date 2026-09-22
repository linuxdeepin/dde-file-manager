// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// 覆盖函数清单(view/collectionframe.cpp) -> 用例映射:
// - CollectionFramePrivate::updateStretchRect -> UpdateStretchRect_AfterResize_FourEdgesBuilt
// - CollectionFramePrivate::updateMoveRect -> UpdateMoveRect_WithAndWithoutTitleBar (分支: 无titleBar早退)
// - CollectionFramePrivate::getCurrentResponseArea -> ResponseArea_EdgesAndTitleBar_Detected
//       (分支: Left/Top 叠加、四边、TitleBar、UnKnow)
// - CollectionFramePrivate::updateCursorState -> CursorState_StretchAndMove_CursorShapeSet
//       (分支: canStretch 四向/角落; canMove+TitleBar; 默认 Arrow)
// - CollectionFramePrivate::updateFrameGeometry -> FrameGeometry_NoSurface_EarlyReturn
// - CollectionFramePrivate::moveResultRectPos -> MoveResultRectPos_NoSurface_ReturnsOldTopLeft
// - CollectionFramePrivate::stretchResultRect -> StretchResultRect_NoSurface_ReturnsOldGeometry
// - CollectionFramePrivate::calcLeftX -> CalcLeftX_BelowMin_ClampedToZero (分支: 过小钳0)
// - CollectionFramePrivate::calcRightX -> CalcRightX_ClampedToParentWidth (分支: 无parent/超界)
// - CollectionFramePrivate::calcTopY -> CalcTopY_BelowMin_ClampedToZero
// - CollectionFramePrivate::calcBottomY -> CalcBottomY_ClampedToParentHeight
// 分支说明见各用例注释。

#include "stubext.h"
#include "view/collectionframe.h"
#include "view/collectionframe_p.h"
#include "private/surface.h"

#include <QWidget>
#include <QEvent>

#include <gtest/gtest.h>

using namespace ddplugin_organizer;

namespace {

class TestableCollectionFrame : public CollectionFrame
{
public:
    using CollectionFrame::CollectionFrame;
    CollectionFramePrivate *priv() { return d.data(); }
};

}   // namespace

class UT_CollectionFrameCov : public testing::Test
{
protected:
    void SetUp() override
    {
        frame = new TestableCollectionFrame();
        frame->setCollectionFeatures(static_cast<CollectionFrame::CollectionFrameFeatures>(
                CollectionFrame::CollectionFrameMovable | CollectionFrame::CollectionFrameStretchable));
        frame->resize(800, 600);
    }

    void TearDown() override
    {
        delete frame;
        stub.clear();
    }

public:
    TestableCollectionFrame *frame = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(UT_CollectionFrameCov, UpdateStretchRect_AfterResize_FourEdgesBuilt)
{
    CollectionFramePrivate *d = frame->priv();
    // Arrange: known frame size 800x600
    frame->resize(800, 600);

    // Act
    d->updateStretchRect();

    // Assert: four edges in fixed order, sized by threshold
    ASSERT_EQ(d->stretchRects.count(), 4);
    EXPECT_EQ(d->stretchRects.at(0).height(), 600);   // left band spans full height
    EXPECT_EQ(d->stretchRects.at(2).x() + d->stretchRects.at(2).width(), 800);   // right band ends at width
}

TEST_F(UT_CollectionFrameCov, UpdateMoveRect_WithAndWithoutTitleBar)
{
    CollectionFramePrivate *d = frame->priv();
    // Arrange: no title bar widget yet, titleBarRect preset to a sentinel
    d->titleBarWidget = nullptr;
    d->titleBarRect = QRect(1, 2, 3, 4);

    // Act: branch without titleBarWidget keeps sentinel
    d->updateMoveRect();

    // Assert: early return keeps the old value
    EXPECT_EQ(d->titleBarRect, QRect(1, 2, 3, 4));

    // Arrange: attach a title bar widget with its own geometry
    QWidget titleBar(frame);
    titleBar.setGeometry(QRect(10, 20, 200, 30));
    d->titleBarWidget = &titleBar;

    // Act
    d->updateMoveRect();

    // Assert: rect synced from widget
    EXPECT_EQ(d->titleBarRect, QRect(10, 20, 200, 30));
    d->titleBarWidget = nullptr;
}

TEST_F(UT_CollectionFrameCov, ResponseArea_EdgesAndTitleBar_Detected)
{
    CollectionFramePrivate *d = frame->priv();
    // Arrange: 800x600 frame with stretch bands and a title band
    frame->resize(800, 600);
    d->updateStretchRect();
    d->titleBarRect = QRect(0, 0, 800, 40);
    const QPoint cornerPos(1, 1);
    const QPoint titlePos(400, 20);
    const QPoint centerPos(400, 300);

    // Act
    auto corner = d->getCurrentResponseArea(cornerPos);
    auto title = d->getCurrentResponseArea(titlePos);
    auto center = d->getCurrentResponseArea(centerPos);

    // Assert: corner overlays two bands, title and center map to their areas
    const int expectedCorner = CollectionFramePrivate::LeftRect | CollectionFramePrivate::TopRect;
    EXPECT_EQ(static_cast<int>(corner), expectedCorner);
    EXPECT_EQ(static_cast<int>(title), static_cast<int>(CollectionFramePrivate::TitleBarRect));
    EXPECT_EQ(static_cast<int>(center), static_cast<int>(CollectionFramePrivate::UnKnowRect));
    EXPECT_NE(static_cast<int>(center), static_cast<int>(CollectionFramePrivate::TitleBarRect));
}

TEST_F(UT_CollectionFrameCov, CursorState_StretchAndMove_CursorShapeSet)
{
    CollectionFramePrivate *d = frame->priv();
    // Arrange: features allow stretch and move (set in SetUp)
    const auto &leftArea = CollectionFramePrivate::LeftRect;
    const auto &topArea = CollectionFramePrivate::TopRect;
    const auto &titleArea = CollectionFramePrivate::TitleBarRect;
    const auto &unknownArea = CollectionFramePrivate::UnKnowRect;

    // Act
    d->updateCursorState(leftArea);
    const Qt::CursorShape leftShape = frame->cursor().shape();
    d->updateCursorState(topArea);
    const Qt::CursorShape topShape = frame->cursor().shape();
    d->updateCursorState(titleArea);
    const Qt::CursorShape titleShape = frame->cursor().shape();
    d->updateCursorState(unknownArea);
    const Qt::CursorShape unknownShape = frame->cursor().shape();

    // Assert: each response area maps to its resize/move cursor
    EXPECT_EQ(leftShape, Qt::SizeHorCursor);
    EXPECT_EQ(topShape, Qt::SizeVerCursor);
    EXPECT_EQ(titleShape, Qt::SizeAllCursor);
    EXPECT_EQ(unknownShape, Qt::ArrowCursor);
}

TEST_F(UT_CollectionFrameCov, FrameGeometry_NoSurface_EarlyReturn)
{
    CollectionFramePrivate *d = frame->priv();
    // Arrange: frame without a Surface parent, fixed geometry
    const QRect geo(10, 10, 300, 200);
    frame->setGeometry(geo);
    ASSERT_EQ(frame->geometry(), geo);

    // Act
    d->updateFrameGeometry();

    // Assert: without a surface the frame geometry collapses to empty,
    // so the widget is never laid out in an invalid parent
    EXPECT_TRUE(frame->geometry().isEmpty());
    EXPECT_EQ(frame->size(), QSize(0, 0));
}

TEST_F(UT_CollectionFrameCov, MoveResultRectPos_NoSurface_ReturnsOldTopLeft)
{
    CollectionFramePrivate *d = frame->priv();
    // Arrange: record an old geometry; frame has no surface
    d->oldGeometry = QRect(10, 20, 100, 50);
    bool validPos = true;

    // Act
    QPoint pos = d->moveResultRectPos(&validPos);

    // Assert: falls back to old geometry top-left
    EXPECT_EQ(pos, QPoint(10, 20));
    EXPECT_TRUE(validPos);
}

TEST_F(UT_CollectionFrameCov, StretchResultRect_NoSurface_ReturnsOldGeometry)
{
    CollectionFramePrivate *d = frame->priv();
    // Arrange: frame without surface
    d->oldGeometry = QRect(5, 6, 120, 80);

    // Act
    QRect result = d->stretchResultRect();

    // Assert: unchanged old geometry returned
    EXPECT_EQ(result, QRect(5, 6, 120, 80));
    EXPECT_EQ(result.size(), QSize(120, 80));
}

TEST_F(UT_CollectionFrameCov, CalcLeftX_BelowMin_ClampedToZero)
{
    CollectionFramePrivate *d = frame->priv();
    // Arrange: stretch end dragged left beyond the screen
    d->oldGeometry = QRect(0, 0, 800, 600);
    d->stretchEndPoint = QPoint(-50, 100);

    // Act
    int x = d->calcLeftX();

    // Assert: clamped to min limit 0
    EXPECT_EQ(x, 0);
    EXPECT_EQ(d->stretchEndPoint.x(), -50);
}

TEST_F(UT_CollectionFrameCov, CalcRightX_ClampedToParentWidth)
{
    CollectionFramePrivate *d = frame->priv();
    // Arrange: parent widget limits the right edge
    QWidget parentHolder;
    parentHolder.resize(500, 400);
    frame->setParent(&parentHolder);
    frame->setGeometry(QRect(0, 0, 100, 100));
    d->oldGeometry = QRect(0, 0, 100, 100);
    d->stretchEndPoint = QPoint(900, 50);

    // Act
    int x = d->calcRightX();

    // Assert: clamped to parent width 500
    EXPECT_EQ(x, 500);
    EXPECT_LE(x, d->stretchEndPoint.x());

    // Cleanup: detach from the stack parent to avoid double deletion
    frame->setParent(nullptr);
}

TEST_F(UT_CollectionFrameCov, CalcTopY_BelowMin_ClampedToZero)
{
    CollectionFramePrivate *d = frame->priv();
    // Arrange: stretch end dragged above the screen
    d->oldGeometry = QRect(0, 0, 800, 600);
    d->stretchEndPoint = QPoint(100, -80);

    // Act
    int y = d->calcTopY();

    // Assert: clamped to min limit 0
    EXPECT_EQ(y, 0);
    EXPECT_EQ(d->stretchEndPoint.y(), -80);
}

TEST_F(UT_CollectionFrameCov, CalcBottomY_ClampedToParentHeight)
{
    CollectionFramePrivate *d = frame->priv();
    // Arrange: parent widget limits the bottom edge
    QWidget parentHolder;
    parentHolder.resize(500, 400);
    frame->setParent(&parentHolder);
    frame->setGeometry(QRect(0, 0, 100, 100));
    d->oldGeometry = QRect(0, 0, 100, 100);
    d->stretchEndPoint = QPoint(50, 800);

    // Act
    int y = d->calcBottomY();

    // Assert: clamped to parent height 400
    EXPECT_EQ(y, 400);
    EXPECT_LE(y, d->stretchEndPoint.y());

    // Cleanup: detach from the stack parent to avoid double deletion
    frame->setParent(nullptr);
}
