/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     wangchunlin<wangchunlin@uniontech.com>
 *
 * Maintainer: wangchunlin<wangchunlin@uniontech.com>
 *             xinglinkun<xinglinkun@uniontech.com>
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
#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>
#include <QTest>
#include <QScreen>
#include <QEventLoop>
#include <QVBoxLayout>
#include <QAbstractAnimation>
#include <qabstractslider.h>

#define private public
#define protected public

#include <QWidget>
#include "../dde-wallpaper-chooser/wallpaperitem.h"
#include "../dde-wallpaper-chooser/frame.h"
#include "../dde-wallpaper-chooser/wallpaperlist.h"
#include "stubext.h"

using namespace testing;

namespace  {
       class WallpaperlistTest : public Test {
       public:

           void SetUp() override {
               for(int i = 0; i < 3; ++i) {
                   m_list->addItem(&item[i]);
               }
           }

           void TearDown() override {
           }

       private:
           WallpaperList list;
           WallpaperList* m_list = &list;
           WallpaperItem item[3];
       };
}

TEST_F(WallpaperlistTest, test_count)
{
    int ret = m_list->count();
    EXPECT_EQ(3, ret);
}

TEST_F(WallpaperlistTest, test_keypressevent)
{
    QKeyEvent* eventright = new QKeyEvent(QEvent::KeyPress, Qt::Key_Right, Qt::NoModifier);
    QKeyEvent* eventleft = new QKeyEvent(QEvent::KeyPress, Qt::Key_Left, Qt::NoModifier);
    QKeyEvent* eventdown = new QKeyEvent(QEvent::KeyPress, Qt::Key_Down, Qt::NoModifier);
    stub_ext::StubExt stu;
    stu.set_lamda(ADDR(WallpaperList, setCurrentIndex), [](){return;});
    bool index = false;
    stu.reset(&WallpaperList::setCurrentIndex);
    stu.set_lamda(ADDR(WallpaperList, setCurrentIndex), [&index](){ index = true; return;});
    m_list->keyPressEvent(eventright);
    EXPECT_TRUE(index);

    m_list->keyPressEvent(eventleft);
    EXPECT_TRUE(index);

    bool ignore = false;
    stu.set_lamda(ADDR(QKeyEvent, ignore), [&ignore](){ignore = true;});
    m_list->keyPressEvent(eventdown);
    EXPECT_TRUE(ignore);

    bool judge = false;
    stu.set_lamda(ADDR(QKeyEvent, isAutoRepeat), [&index](){index = true; return true;});
    stu.set_lamda(ADDR(QKeyEvent, accept), [&judge](){ judge = true;});
    stu.set_lamda(ADDR(QAbstractAnimation, state), [](){ return QAbstractAnimation::State::Running;});
    m_list->keyPressEvent(eventleft);
    EXPECT_TRUE(judge);

    delete eventright;
    delete eventleft;
    delete eventdown;
}

TEST_F(WallpaperlistTest, test_removewallpaper)
{
   int count = m_list->count();
   stub_ext::StubExt stu;
   static QString temp("test");
   stu.set_lamda(ADDR(QLayout, removeWidget), [](){return;});
   stu.set_lamda(ADDR(WallpaperItem, getPath), [](){return temp;});
   stu.set_lamda(ADDR(QObject, deleteLater), [](){return;});
   m_list->removeWallpaper(QString("test"));
   EXPECT_NE(count, m_list->count());

   if (m_list->m_items.size())
       m_list->prevItem = m_list->m_items[0];
   m_list->removeWallpaper(QString("test"));
   EXPECT_NE(count, m_list->count());

   if (m_list->m_items.size())
       m_list->nextItem = m_list->m_items[0];
   m_list->removeWallpaper(QString("test"));
   EXPECT_NE(count, m_list->count());
}

TEST_F(WallpaperlistTest, test_wallpaperitempressed)
{
   stub_ext::StubExt stu;
   bool judge = false;
   stu.set_lamda(ADDR(WallpaperList, setCurrentIndex), [&judge](){judge = true;return;});
   m_list->wallpaperItemPressed();
   EXPECT_TRUE(judge);

   QWheelEvent event(QPointF(), 1, Qt::LeftButton, Qt::NoModifier);
   m_list->wheelEvent(&event);//函数体无代码,直接调用
   m_list->wallpaperItemHoverOut();//函数体无代码

   bool deleteButton = false;
   stu.set_lamda(ADDR(WallpaperList, showDeleteButtonForItem), [&deleteButton](){deleteButton = true;});

   auto item = qobject_cast<WallpaperItem *>(m_list->item(0));
   ASSERT_NE(item, nullptr);

   stu.set_lamda(ADDR(WallpaperItem, isVisible), [](){return true;});
   stu.set_lamda(ADDR(WallpaperList, sender), [item]()->QObject *{return item;});

   m_list->wallpaperItemHoverIn();
   EXPECT_TRUE(deleteButton);
}

TEST_F(WallpaperlistTest, test_scrolllist)
{
    stub_ext::StubExt stu;
    m_list->scrollList(2, 200);
    bool isstate = false;
    bool ismini = false;
    stu.set_lamda(ADDR(QAbstractAnimation, state), [&isstate](){isstate = true; return QAbstractAnimation::Stopped;});
    stu.set_lamda(ADDR(QAbstractSlider, minimum), [&ismini](){ismini = true; return -1;});
    auto itemOne = new WallpaperItem;
    auto itemTwo = new WallpaperItem;
    if (m_list->prevItem == nullptr) {
        m_list->prevItem = itemOne;
    }
    if(m_list->nextItem == nullptr) {
        m_list->nextItem = itemTwo;
    }
    m_list->scrollList(2, 200);
    EXPECT_TRUE(isstate);
    EXPECT_TRUE(ismini);
    bool isrun = false;
    stu.reset(&QAbstractAnimation::state);
    stu.set_lamda(ADDR(QAbstractAnimation, state), [&isrun](){isrun = true; return QAbstractAnimation::Running;});
    m_list->scrollList(2, 200);
    EXPECT_TRUE(isrun);

    delete itemOne;
    delete itemTwo;
    itemOne = nullptr;
    itemTwo = nullptr;
}

TEST_F(WallpaperlistTest, test_resizeevent)
{
    stub_ext::StubExt stu;
    stu.set_lamda(VADDR(Frame, resizeEvent), [](){return;});
    bool iswidth = false;
    QResizeEvent event(QSize(200, 200), QSize(100, 100));
    if (m_list->m_contentLayout)
        m_list->resizeEvent(&event);

    QSize first = m_list->m_gridSize;
    stu.set_lamda(ADDR(QWidget, width), [&iswidth](){iswidth = true; return 100;});
    m_list->resizeEvent(&event);
    if (m_list->m_contentLayout) {
        QSize second = m_list->m_gridSize;
        EXPECT_NE(first, second);
    }
    EXPECT_TRUE(iswidth);
}

TEST_F(WallpaperlistTest, test_addwallpaper)
{
    stub_ext::StubExt stu;
    m_list->updateItemThumb();
    stu.set_lamda(ADDR(WallpaperList, addItem), [](){return;});
    WallpaperItem* item = m_list->addWallpaper("/");
    bool ispress = false;
    bool ishoverin = false;
    bool ishoverout = false;
    QObject::connect(item, &WallpaperItem::pressed, item, [&ispress](){ispress = true;});
    QObject::connect(item, &WallpaperItem::hoverIn, item, [&ishoverin](){ishoverin = true;});
    QObject::connect(item, &WallpaperItem::hoverOut, item, [&ishoverout](){ishoverout = true;});

    emit item->pressed();
    emit item->hoverIn();
    emit item->hoverOut();

    EXPECT_TRUE(ispress);
    EXPECT_TRUE(ishoverin);
    EXPECT_TRUE(ishoverout);
}

TEST_F(WallpaperlistTest, test_getCurrentItem)
{
    m_list->m_index = 0;
    WallpaperItem* item = m_list->m_items.at(0);
    if (m_list->m_items.size()) {
        WallpaperItem* ret = m_list->getCurrentItem();
        EXPECT_TRUE(item == ret);
    }
}

TEST_F(WallpaperlistTest, test_nextpage)
{
    stub_ext::StubExt stu;
    m_list->nextPage();
    bool iswidth = false;
    bool isscro = false;
    stu.set_lamda(ADDR(QSize, width), [&iswidth](){iswidth = true; return 100;});
    stu.set_lamda(ADDR(WallpaperList, scrollList), [&isscro](){isscro = true; return;});
    m_list->nextPage();
    EXPECT_TRUE(iswidth);
    EXPECT_TRUE(isscro);
}

TEST_F(WallpaperlistTest, test_prevPage)
{
    stub_ext::StubExt stu;
    m_list->prevPage();
    bool iswidth = false;
    bool isscro = false;
    stu.set_lamda(ADDR(QSize, width), [&iswidth](){iswidth = true; return 100;});
    stu.set_lamda(ADDR(WallpaperList, scrollList), [&isscro](){isscro = true; return;});
    m_list->nextPage();
    EXPECT_TRUE(iswidth);
    EXPECT_TRUE(isscro);
}

TEST_F(WallpaperlistTest, test_showevent)
{
    stub_ext::StubExt stu;
    QShowEvent event;
    bool isup = false;
    stu.set_lamda(ADDR(WallpaperList, updateBothEndsItem), [&isup](){isup = true; return;});
    m_list->showEvent(&event);
    EXPECT_TRUE(isup);
}

TEST_F(WallpaperlistTest, test_wallpaperlist)
{
    WallpaperList* list = new WallpaperList;
    delete list;
    list = nullptr;
}

TEST_F(WallpaperlistTest, test_item)
{
    int count = m_list->m_contentLayout->count();
    QWidget* ret = m_list->item(count);
    EXPECT_TRUE(ret == nullptr);
    ret = m_list->item(0);
    QWidget* compare = m_list->m_contentLayout->itemAt(0)->widget();
    EXPECT_TRUE(compare == ret);
}

TEST_F(WallpaperlistTest, test_setcurrentindex)
{
    stub_ext::StubExt stu;
    m_list->setCurrentIndex(-1);
    bool isslideup = false;
    bool isslidedown = false;
    stu.set_lamda(ADDR(WallpaperItem, slideUp), [&isslideup](){isslideup = true; return;});
    stu.set_lamda(ADDR(WallpaperItem, slideDown), [&isslidedown](){isslidedown = true; return;});
    m_list->setCurrentIndex(0);
    EXPECT_TRUE(m_list->m_index == 0);
    EXPECT_TRUE(isslideup);
    EXPECT_TRUE(isslidedown);
}

TEST_F(WallpaperlistTest, test_showDeleteButtonForItem)
{
    stub_ext::StubExt stu;
    WallpaperItem* item = new WallpaperItem;
    bool isdelete = false;
    bool isemitsignal = false;
    stu.set_lamda(ADDR(WallpaperItem, getDeletable), [&isdelete](){isdelete = true; return true;});
    m_list->showDeleteButtonForItem(item);
    EXPECT_TRUE(isdelete);

    stu.set_lamda(ADDR(QRect, isNull), [](){return false;});
    QObject::connect(m_list, &WallpaperList::mouseOverItemChanged, m_list, [&isemitsignal](){isemitsignal = true;});
    m_list->showDeleteButtonForItem(item);

    EXPECT_TRUE(isemitsignal);

    stu.reset(&WallpaperItem::getDeletable);
    stu.set_lamda(ADDR(WallpaperItem, getDeletable), [&isdelete](){isdelete = true; return false;});
    m_list->showDeleteButtonForItem(item);

    EXPECT_TRUE(isemitsignal);

    delete item;
    item = nullptr;
}

TEST_F(WallpaperlistTest, test_clear)
{
    m_list->clear();
    EXPECT_TRUE(m_list->m_items.isEmpty());
    EXPECT_TRUE(m_list->prevItem == nullptr);
    EXPECT_TRUE(m_list->nextItem == nullptr);
}

TEST_F(WallpaperlistTest, test_updateBothEndsItem)
{
    stub_ext::StubExt stu;
    stu.set_lamda(ADDR(QAbstractSlider, value), [](){return 0;});
    m_list->updateBothEndsItem();
    EXPECT_EQ(m_list->prevItem, nullptr);
    EXPECT_EQ(m_list->nextItem, nullptr);


    stu.set_lamda(ADDR(WallpaperItem, initUI), [](){return;});
    stu.set_lamda(ADDR(WallpaperItem, initAnimation), [](){return;});
    WallpaperItem* item = new WallpaperItem;
    stu.set_lamda(ADDR(WallpaperList, item), [item](){return item;});
    stu.set_lamda(ADDR(QAbstractSlider, minimum), [](){return 1;});
    stu.set_lamda(ADDR(QAbstractSlider, maximum), [](){return 2;});
    stu.set_lamda(ADDR(QSize, width), [](){return 10;});
    stu.set_lamda(ADDR(WallpaperItem, setOpacity), [](){return;});
    m_list->updateBothEndsItem();
    EXPECT_EQ(m_list->prevItem, item);
    EXPECT_EQ(m_list->nextItem, item);
    delete item;
}
