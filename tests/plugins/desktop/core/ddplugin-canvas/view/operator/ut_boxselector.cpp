// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "view/operator/boxselector.h"
#include "view/canvasview_p.h"

#include "stubext.h"

#include <gtest/gtest.h>

#include <QPaintEvent>

using namespace testing;
using namespace ddplugin_canvas;

TEST(RubberBand, construct)
{
    RubberBand rb;
    EXPECT_FALSE(rb.autoFillBackground());
    EXPECT_TRUE(rb.testAttribute(Qt::WA_TranslucentBackground));
}

TEST(RubberBand, touch)
{
    stub_ext::StubExt stub;
    bool hide = false;
    stub.set_lamda(&QWidget::hide, [&hide](){
        hide = true;
    });

    RubberBand rb;
    QWidget wid;
    rb.touch(&wid);

    EXPECT_EQ(rb.parentWidget(), &wid);
    EXPECT_TRUE(hide);

    hide = false;
    QWidget wid2;
    rb.touch(&wid2);
    EXPECT_EQ(rb.parentWidget(), &wid2);
    EXPECT_TRUE(hide);
}

TEST(RubberBand, onParentDestroyed)
{
    stub_ext::StubExt stub;
    bool hide = false;
    stub.set_lamda(&QWidget::hide, [&hide](){
        hide = true;
    });

    QWidget wid;
    RubberBand rb;
    rb.setParent(&wid);

    rb.onParentDestroyed(nullptr);
    EXPECT_EQ(rb.parentWidget(), &wid);
    EXPECT_FALSE(hide);

    hide = false;
    rb.onParentDestroyed(&wid);
    EXPECT_EQ(rb.parentWidget(), nullptr);
    EXPECT_TRUE(hide);
}

TEST(RubberBand, paintEvent)
{
    RubberBand rb;
    QPaintEvent e(rb.rect());
    EXPECT_NO_THROW(rb.paintEvent(&e));
}

TEST(BoxSelector, beginSelect)
{
    BoxSelector bs;
    bs.beginSelect(QPoint(1,1), false);

    EXPECT_EQ(bs.begin, QPoint(1,1));
    EXPECT_EQ(bs.end, QPoint(1,1));
    EXPECT_TRUE(bs.active);
    EXPECT_FALSE(bs.automatic);
}

TEST(BoxSelector, set)
{
    BoxSelector bs;
    bool update = false;
    stub_ext::StubExt stub;
    stub.set_lamda(&BoxSelector::delayUpdate, [&update](){
        update = true;
    });

    bs.setAcvite(true);
    EXPECT_TRUE(bs.active);
    EXPECT_TRUE(update);

    update = false;
    bs.setBegin(QPoint(2,2));
    EXPECT_EQ(bs.begin, QPoint(2,2));
    EXPECT_TRUE(update);

    update = false;
    bs.setEnd(QPoint(4,4));
    EXPECT_EQ(bs.begin, QPoint(2,2));
    EXPECT_EQ(bs.end, QPoint(4,4));
    EXPECT_TRUE(update);
}

TEST(BoxSelector, validRect)
{
    BoxSelector bs;
    bs.begin = QPoint(0, 0);
    bs.end = QPoint(1000, 1000);

    CanvasView view;
    view.QAbstractItemView::setGeometry(200, 200, 200, 200);

    EXPECT_EQ(bs.validRect(&view), QRect(0,0,200,200));

    bs.begin = QPoint(300, 300);
    bs.end = QPoint(349, 349);
    EXPECT_EQ(bs.validRect(&view), QRect(100,100,50,50));
}

TEST(BoxSelector, isBeginFrom)
{
    BoxSelector bs;
    bs.begin = QPoint(0, 0);

    CanvasView view;
    view.QAbstractItemView::setGeometry(200, 200, 200, 200);
    EXPECT_FALSE(bs.isBeginFrom(&view));

    bs.begin = QPoint(210, 210);
    EXPECT_TRUE(bs.isBeginFrom(&view));
}

TEST(BoxSelector, endSelect)
{
    stub_ext::StubExt stub;
    bool update = false;
    stub.set_lamda(&BoxSelector::updateRubberBand, [&update](){
        update = true;
    });

    BoxSelector bs;
    bs.active = true;

    bs.endSelect();
    EXPECT_TRUE(update);
    EXPECT_FALSE(bs.active);

    update = false;
    bs.updateTimer.start(1000);
    bs.endSelect();
    EXPECT_FALSE(update);
    EXPECT_FALSE(bs.active);
}

TEST(BoxSelector, eventFilter)
{
    BoxSelector bs;

    stub_ext::StubExt stub;
    bool endSelect = false;
    stub.set_lamda(&BoxSelector::endSelect, [&endSelect](){
        endSelect = true;
    });

    bool update = false;
    stub.set_lamda(&BoxSelector::delayUpdate, [&update](){
        update = true;
    });

    QWidget wid;
    {
        QMouseEvent e(QEvent::MouseButtonRelease, QPoint(0,0), Qt::LeftButton, Qt::LeftButton, {});
        EXPECT_FALSE(bs.eventFilter(&wid, &e));
        EXPECT_FALSE(endSelect);
        EXPECT_FALSE(update);
    }

    bs.active = true;
    {
        update = false;
        endSelect = false;
        QMouseEvent e(QEvent::MouseMove, QPoint(100, 100), QPoint(100, 100), Qt::LeftButton, Qt::LeftButton, {});
        EXPECT_FALSE(bs.eventFilter(&wid, &e));
        EXPECT_FALSE(endSelect);
        EXPECT_TRUE(update);
        EXPECT_EQ(bs.end, QPoint(100, 100));
    }

    {
        update = false;
        endSelect = false;
        QMouseEvent e(QEvent::MouseMove, QPoint(110, 110), QPoint(110, 110), Qt::RightButton, Qt::RightButton, {});
        EXPECT_FALSE(bs.eventFilter(&wid, &e));
        EXPECT_TRUE(endSelect);
        EXPECT_FALSE(update);
        EXPECT_EQ(bs.end, QPoint(100, 100));
    }

    {
        update = false;
        endSelect = false;
        QMouseEvent e(QEvent::MouseButtonRelease, QPoint(110, 110), QPoint(110, 110), Qt::LeftButton, Qt::LeftButton, {});
        EXPECT_FALSE(bs.eventFilter(&wid, &e));
        EXPECT_TRUE(endSelect);
        EXPECT_FALSE(update);
        EXPECT_EQ(bs.end, QPoint(100, 100));
    }
}
