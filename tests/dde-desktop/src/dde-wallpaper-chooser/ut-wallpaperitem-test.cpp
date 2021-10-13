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
#include <QPushButton>
#include <QObject>
#include <QFuture>
#include <QPixmap>
#include <QQueue>

#define private public
#define protected public

#include "../dde-wallpaper-chooser/wallpaperitem.h"
#include "../stub-ext/stubext.h"
#include "../dde-wallpaper-chooser/thumbnailmanager.h"
#include "../dde-wallpaper-chooser/frame.h"

class WrapperWidget;
using namespace testing;

namespace  {
       class WallpaperItemTest : public Test {
       public:

           void SetUp() override {
               if (m_item == nullptr) {
                    m_item = new WallpaperItem;
               }

           }

           void TearDown() override {
               delete m_item;
           }

       private:

           WallpaperItem* m_item = nullptr;
       };
}

TEST_F(WallpaperItemTest, test_keyevent)
{
    QKeyEvent* eventup = new QKeyEvent(QEvent::KeyPress, Qt::Key_Up, Qt::NoModifier);
    QKeyEvent* eventdown = new QKeyEvent(QEvent::KeyPress, Qt::Key_Down, Qt::NoModifier);
    QKeyEvent* eventdefault = new QKeyEvent(QEvent::KeyPress, Qt::Key_0, Qt::NoModifier);

    m_item->addButton("1", "test01", 160, 0, 0, 1, 1);
    m_item->addButton("2", "test02", 160, 1, 0, 1, 1);

    stub_ext::StubExt stu;
    bool judge = false;

    stu.set_lamda(ADDR(QWidget, hasFocus), [](){return true;});
    m_item->keyPressEvent(eventup);
    if (m_item->m_buttonLayout->itemAt(0))
        EXPECT_TRUE(m_item->m_buttonLayout->itemAt(0)->widget()->hasFocus());

    m_item->keyPressEvent(eventdown);
    if (m_item->m_buttonLayout->itemAt(m_item->m_buttonLayout->count() - 1))
        EXPECT_TRUE(m_item->m_buttonLayout->itemAt(m_item->m_buttonLayout->count() - 1)->widget()->hasFocus());

    m_item->focusLastButton();
    if (m_item->m_buttonLayout->itemAt(m_item->m_buttonLayout->count() - 1))
        EXPECT_TRUE(m_item->m_buttonLayout->itemAt(m_item->m_buttonLayout->count() - 1)->widget()->hasFocus());

    stu.set_lamda(ADDR(QKeyEvent, ignore), [&judge](){judge = true;});
    m_item->keyPressEvent(eventdefault);
    EXPECT_TRUE(judge == true);

    delete eventup;
    eventup = nullptr;
    delete eventdown;
    eventdown = nullptr;
    delete eventdefault;
    eventdefault = nullptr;
}

TEST_F(WallpaperItemTest, test_setdata)
{
    m_item->setData(QString("test"));
    EXPECT_EQ(m_item->m_data, QString("test"));
}

TEST_F(WallpaperItemTest, test_getdeleteable)
{
    m_item->setDeletable(false);
    EXPECT_FALSE(m_item->getDeletable());
}

TEST_F(WallpaperItemTest, test_setusethumbnailmanager)
{
   m_item->setUseThumbnailManager(false);
   EXPECT_FALSE(m_item->m_useThumbnailManager);
}

TEST_F(WallpaperItemTest, test_enterEvent)
{
    QEvent event(QEvent::KeyPress);
    bool judge = false;
    QObject::connect(m_item, &WallpaperItem::hoverIn, m_item, [&judge](){judge = true;});
    m_item->enterEvent(&event);
    EXPECT_TRUE(judge);
}

TEST_F(WallpaperItemTest, test_leaveEvent)
{
    QEvent event(QEvent::KeyPress);
    bool judge = false;
    QObject::connect(m_item, &WallpaperItem::hoverOut, m_item, [&judge](){judge = true;});
    m_item->leaveEvent(&event);
    EXPECT_TRUE(judge);
}

TEST_F(WallpaperItemTest, test_onFindAborted)
{
     QQueue<QString> que;
     que.push_back("string");
     stub_ext::StubExt stu;
     bool judge = false;
     stu.set_lamda(ADDR(QList<QString>, contains), [&judge](){judge = true; return true;});
     stu.set_lamda(ADDR(WallpaperItem, refindPixmap), [](){return;});
     m_item->onFindAborted(que);
     EXPECT_TRUE(judge);
}

TEST_F(WallpaperItemTest, test_mousePressEvent)
{
     QMouseEvent event(QEvent::KeyPress, QPointF(), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
     bool judge = false;
     QObject::connect(m_item, &WallpaperItem::pressed, m_item, [&judge](){judge = true;});
     m_item->mousePressEvent(&event);
     EXPECT_TRUE(judge);
}

TEST_F(WallpaperItemTest, test_thumbnailFinished)
{
     stub_ext::StubExt stu;
     stu.set_lamda(ADDR(QFuture<QPixmap>, result), [](){return QPixmap();});
     m_item->thumbnailFinished();
     EXPECT_FALSE(m_item->m_wrapper == nullptr);
}

TEST_F(WallpaperItemTest, test_setpath)
{
    m_item->setPath(QString("test"));
    EXPECT_TRUE(m_item->m_path == QString("test"));

    m_item->setUseThumbnailManager(false);
    m_item->useThumbnailManager();
    EXPECT_EQ(m_item->m_useThumbnailManager, false);
}

TEST_F(WallpaperItemTest, test_initpixmap)
{
    stub_ext::StubExt stu;
    m_item->m_useThumbnailManager = true;
    bool isfind = false;
    stu.set_lamda(ADDR(ThumbnailManager, find), [&isfind](){isfind = true; return;});
    m_item->initPixmap();
    EXPECT_TRUE(isfind);

    m_item->m_useThumbnailManager = false;
    m_item->initPixmap();
}

TEST_F(WallpaperItemTest, test_setOpacity)
{
    stub_ext::StubExt stu;
    m_item->setOpacity(1.0000000001);
    m_item->setOpacity(1.5);
    QResizeEvent event(QSize(200, 200), QSize(100, 100));
    m_item->resizeEvent(&event);
    m_item->onThumbnailFounded("1", QPixmap(200, 200));
}

TEST_F(WallpaperItemTest, test_slideup)
{
    stub_ext::StubExt stu;
    bool isy = false;
    bool isstate = false;
    m_item->addButton("desktop", "test01", 160, 0, 0, 1, 1);
    m_item->slideUp();
    if (m_item->m_buttonLayout->itemAt(0))
        EXPECT_EQ(m_item->m_buttonLayout->itemAt(0)->widget()->focusPolicy(), Qt::StrongFocus);

    stu.set_lamda(ADDR(QWidget, y), [&isy](){isy = true; return -1;});
    stu.set_lamda(ADDR(QAbstractAnimation, state), [&isstate](){isstate = true; return QAbstractAnimation::Stopped;});
    if (m_item->m_buttonLayout->count())
        m_item->slideUp();
    EXPECT_TRUE(isy);
    EXPECT_TRUE(isstate);
}

TEST_F(WallpaperItemTest, test_slidedown)
{
    stub_ext::StubExt stu;
    bool isy = false;
    bool isstate = false;
    m_item->addButton("desktop", "test01", 160, 0, 0, 1, 1);
    m_item->slideDown();
    if (m_item->m_buttonLayout->itemAt(0))
        EXPECT_EQ(m_item->m_buttonLayout->itemAt(0)->widget()->focusPolicy(), Qt::NoFocus);

    stu.set_lamda(ADDR(QWidget, y), [&isy](){isy = true; return 0;});
    stu.set_lamda(ADDR(QAbstractAnimation, state), [&isstate](){isstate = true; return QAbstractAnimation::Stopped;});
    m_item->slideDown();
    EXPECT_TRUE(isy);
    EXPECT_TRUE(isstate);
}

TEST_F(WallpaperItemTest, test_getpath)
{
    QString path("usr/bin");
    m_item->m_path = path;
    QString ret = m_item->getPath();
    EXPECT_EQ(ret, "");
    QString path2("/usr/bin");
    m_item->m_path = path2;
    ret = m_item->getPath();
    EXPECT_EQ(path2, ret);
}

TEST_F(WallpaperItemTest, test_data)
{
    QString temp = m_item->m_data;
    QString ret = m_item->data();
    EXPECT_EQ(temp, ret);
}

TEST_F(WallpaperItemTest, test_addbutton)
{
    QString contant("test01");
    int count = m_item->m_buttonLayout->count();
    QPushButton* btn = m_item->addButton("desktop", contant);
    int secount = m_item->m_buttonLayout->count();
    emit btn->clicked();
    EXPECT_EQ(btn->text(), contant);
    EXPECT_NE(count, secount);

    QString contant2("test02000000000000000000000000000000000000000000000000000");
    int count2 = m_item->m_buttonLayout->count();
    QPushButton* btn2 = m_item->addButton("desktop", contant2);
    int secount2 = m_item->m_buttonLayout->count();

    emit btn2->clicked();
    EXPECT_NE(btn2->text(), contant2);
    EXPECT_NE(count2, secount2);
}

TEST_F(WallpaperItemTest, test_addbutton_layout)
{
    QString contant("test01");
    int count = m_item->m_buttonLayout->count();
    QPushButton* btn = m_item->addButton("desktop", contant, 160, 0, 0, 1, 2);
    int secount = m_item->m_buttonLayout->count();
    emit btn->clicked();
    EXPECT_EQ(btn->text(), contant);
    EXPECT_NE(count, secount);

    QString contant2("test02000000000000000000000000000000000000000000000000000");
    int count2 = m_item->m_buttonLayout->count();
    QPushButton* btn2 = m_item->addButton("desktop", contant2, 80, 0, 0, 1, 1);
    int secount2 = m_item->m_buttonLayout->count();

    emit btn2->clicked();
    EXPECT_NE(btn2->text(), contant2);
    EXPECT_NE(count2, secount2);
}

TEST_F(WallpaperItemTest, test_eventfilter)
{
    QKeyEvent eventtab(QEvent::KeyPress, Qt::Key_Tab, Qt::NoModifier);
    QKeyEvent eventbacktab(QEvent::KeyPress, Qt::Key_Backtab, Qt::NoModifier);
    QKeyEvent eventother(QEvent::MouseMove, Qt::Key_Backtab, Qt::NoModifier);

    bool ret = m_item->eventFilter(m_item, &eventtab);
    EXPECT_TRUE(ret);
    ret = m_item->eventFilter(m_item, &eventbacktab);
    EXPECT_TRUE(ret);
    ret = m_item->eventFilter(m_item, &eventother);
    EXPECT_FALSE(ret);
}
