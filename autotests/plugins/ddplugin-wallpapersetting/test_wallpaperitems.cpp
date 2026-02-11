// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <stubext.h>
#include <QApplication>
#include <QKeyEvent>
#include <QPushButton>
#include <QWidget>
#include <QScrollBar>

#include "wallpaperitem.h"
#include "wallpaperlist.h"

using namespace ddplugin_wallpapersetting;

class UT_WallpaperItemList : public testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override { stub.clear(); }
    stub_ext::StubExt stub;
};

// [WallpaperItem]_[RenderAndButtons]_[SignalsAndGeometry]
TEST_F(UT_WallpaperItemList, WallpaperItem_RenderAndButtons_SignalsAndGeometry)
{
    WallpaperItem item;
    item.setItemData("file:///tmp/mock.jpg");
    item.setSketch("file:///tmp/mock.jpg");
    item.setEnableThumbnail(false);
    item.setDeletable(true);

    // Add buttons and verify click emits signal
    auto btn1 = item.addButton("desktop", "Desktop", 80, 0, 0, 1, 1);
    auto btn2 = item.addButton("lock", "Lock Screen", 80, 0, 1, 1, 1);

    QString received;
    QObject::connect(&item, &WallpaperItem::buttonClicked, [&](WallpaperItem *, const QString &id) { received = id; });
    QMetaObject::invokeMethod(btn1, "clicked", Qt::DirectConnection);
    EXPECT_EQ(received, QString("desktop"));

    // Render pixmap path without thumbnail (pure QIcon path)
    EXPECT_NO_THROW(item.renderPixmap());

    // Keyboard navigation coverage
    QKeyEvent keyUp(QEvent::KeyPress, Qt::Key_Up, Qt::NoModifier);
    QApplication::sendEvent(&item, &keyUp);
    QKeyEvent keyDown(QEvent::KeyPress, Qt::Key_Down, Qt::NoModifier);
    QApplication::sendEvent(&item, &keyDown);

    // Cover enter/leave events
    QEvent enterEvent(QEvent::Enter);
    QApplication::sendEvent(&item, &enterEvent);
    QEvent leaveEvent(QEvent::Leave);
    QApplication::sendEvent(&item, &leaveEvent);
}

// [WallpaperList]_[AddRemoveScroll]_[LayoutAndSignals]
TEST_F(UT_WallpaperItemList, WallpaperList_AddRemoveScroll_LayoutAndSignals)
{
    WallpaperList list;
    list.resize(800, 120);
    list.show();

    WallpaperItem *a = list.addItem("file:///a.jpg");
    a->setSketch("file:///a.jpg");
    a->setEnableThumbnail(false);
    WallpaperItem *b = list.addItem("file:///b.jpg");
    b->setSketch("file:///b.jpg");
    b->setEnableThumbnail(false);

    // Select and scroll
    list.setCurrentIndex(0);
    list.nextPage();
    list.prevPage();

    // Remove and ensure not crash
    list.removeItem("file:///a.jpg");
    list.clear();

    // Exercise wheelEvent path (no-op)
    QWheelEvent we(QPointF(0,0), QPointF(0,0), QPoint(0, 120), QPoint(0, 120), Qt::NoButton, Qt::NoModifier, Qt::ScrollBegin, false);
    QApplication::sendEvent(&list, &we);
}

// [WallpaperList]_[MaskWidget_SetRemove]_[ToggleContent]
TEST_F(UT_WallpaperItemList, WallpaperList_MaskWidget_SetRemove_ToggleContent)
{
    WallpaperList list;
    list.resize(600, 120);
    list.show();

    // setMaskWidget with nullptr -> early return
    list.setMaskWidget(nullptr);

    // Set a real mask widget and then remove
    QWidget *mask = new QWidget;
    list.setMaskWidget(mask);
    EXPECT_EQ(list.widget(), mask);

    QWidget *removed = list.removeMaskWidget();
    EXPECT_EQ(removed, mask);
    delete removed;
}

// [WallpaperList]_[KeyRepeat_AnimationRunning]_[AcceptedNoMove]
TEST_F(UT_WallpaperItemList, WallpaperList_KeyRepeat_AnimationRunning_Accepted)
{
    WallpaperList list;
    list.resize(800, 120);
    list.show();

    // Add items and start scroll animation by changing current index
    WallpaperItem *a = list.addItem("file:///a.jpg");
    a->setSketch("file:///a.jpg");
    a->setEnableThumbnail(false);
    WallpaperItem *b = list.addItem("file:///b.jpg");
    b->setSketch("file:///b.jpg");
    b->setEnableThumbnail(false);

    list.setCurrentIndex(0);

    // Build an auto-repeat key event while animation is running
    QKeyEvent ev(QEvent::KeyPress, Qt::Key_Right, Qt::NoModifier, QString(), true, 2);
    QApplication::sendEvent(&list, &ev);

    SUCCEED();
}

// [WallpaperList]_[ShowEvent_UpdateBothEnds_MinMax]
TEST_F(UT_WallpaperItemList, WallpaperList_ShowEvent_UpdateBothEnds_MinMax)
{
    WallpaperList list;
    list.resize(800, 120);
    list.show();

    // Add some items
    for (int i = 0; i < 5; ++i) {
        auto it = list.addItem(QString("file:///i%1.jpg").arg(i));
        it->setSketch("file:///a.jpg");
        it->setEnableThumbnail(false);
    }

    // Force scrollbar to min and trigger showEvent path
    list.horizontalScrollBar()->setValue(list.horizontalScrollBar()->minimum());
    QShowEvent ev;
    QApplication::sendEvent(&list, &ev);

    // Move to maximum and re-evaluate updateBothEndsItem via explicit call
    list.horizontalScrollBar()->setValue(list.horizontalScrollBar()->maximum());
    EXPECT_NO_THROW(list.updateBothEndsItem());
}
