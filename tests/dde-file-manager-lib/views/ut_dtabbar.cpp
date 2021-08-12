/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liqiang<liqianga@uniontech.com>
 *
 * Maintainer: liqiang<liqianga@uniontech.com>
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

#include "stub.h"
#define private public
#define protected public

#include "dfmevent.h"

#include <QTimer>
#include <views/dtabbar.h>
#include <views/dfileview.h>


TEST(Tab, set_tab_text)
{
    Tab temp(nullptr, nullptr);
    temp.setTabText("test");
    EXPECT_TRUE("test" == temp.m_tabText);
}

TEST(Tab, tab_text)
{
    Tab temp(nullptr, nullptr);
    temp.setTabText("test");
    EXPECT_TRUE(temp.m_tabText == temp.tabText());
}


TEST(Tab, file_view)
{
    DFileView *testView = new DFileView();
    Tab temp(nullptr, testView);
    EXPECT_TRUE(testView == temp.fileView());
    delete testView;
    testView = nullptr;
}

TEST(Tab, set_file_view)
{
    DFileView *testView = new DFileView();
    Tab temp(nullptr, nullptr);
    EXPECT_TRUE(nullptr == temp.fileView());
    temp.setFileView(testView);
    EXPECT_TRUE(testView == temp.fileView());
    delete testView;
}

TEST(Tab, current_url)
{
    Tab temp(nullptr, nullptr);
    temp.m_url = DUrl("/home/utTest");
    EXPECT_TRUE(DUrl("/home/utTest") == temp.currentUrl());
}

TEST(Tab, set_current_url)
{
    Tab temp(nullptr, nullptr);
    temp.setCurrentUrl(DUrl("/home/utTest"));
    EXPECT_TRUE(DUrl("/home/utTest") == temp.currentUrl());
}

TEST(Tab, set_fixed_size)
{
    Tab temp(nullptr, nullptr);
    temp.setFixedSize(QSize(20, 20));
    EXPECT_TRUE(QSize(20, 20) == QSize(temp.m_width, temp.m_height));
}

TEST(Tab, set_geometry)
{
    Tab temp(nullptr, nullptr);
    temp.setGeometry(QRect(20, 20, 20, 20));
    EXPECT_TRUE(QRect(20, 20, 20, 20) == temp.geometry());
}

TEST(Tab, geometry)
{
    Tab temp(nullptr, nullptr);
    temp.setGeometry(QRect(20, 20, 20, 20));
    EXPECT_TRUE(QRect(static_cast<int>(temp.x()), static_cast<int>(temp.y()), temp.m_width, temp.m_height) == temp.geometry());
}

TEST(Tab, width)
{
    Tab temp(nullptr, nullptr);
    temp.setFixedSize(QSize(20, 20));
    EXPECT_TRUE(20 == temp.width());
}

TEST(Tab, height)
{
    Tab temp(nullptr, nullptr);
    temp.setFixedSize(QSize(20, 20));
    EXPECT_TRUE(20 == temp.height());
}

TEST(Tab, is_dragging)
{
    Tab temp(nullptr, nullptr);
    temp.m_isDragging = true;
    EXPECT_TRUE(true == temp.isDragging());
}

TEST(Tab, set_hovered)
{
    Tab temp(nullptr, nullptr);
    temp.setHovered(true);
    EXPECT_TRUE(true == temp.m_hovered);
}

TEST(Tab, is_drag_out_side)
{
    Tab temp(nullptr, nullptr);
    temp.m_dragOutSide = true;
    EXPECT_TRUE(true == temp.isDragOutSide());
}

static bool stubRet = false;
TEST(Tab, to_pixmap)
{
    Stub stub;
    auto utStub = (void(*)(const QPainterPath &))([](const QPainterPath &path){
        Q_UNUSED(path);
        stubRet = true;
    });
    stub.set(ADDR(QPainter, drawPath), utStub);
    Tab temp(nullptr, nullptr);

    auto pixFalse = temp.toPixmap(false);
    EXPECT_FALSE(stubRet);
    auto pixTrue = temp.toPixmap(true);
    EXPECT_TRUE(stubRet);
}

TEST(Tab, is_checked)
{
    Tab temp(nullptr, nullptr);
    temp.m_checked = true;
    EXPECT_TRUE(true == temp.isChecked());
}

TEST(Tab, set_checked)
{
    Tab temp(nullptr, nullptr);
    temp.setChecked(false);
    EXPECT_TRUE(false == temp.isChecked());
}

TEST(Tab, border_left)
{
    Tab temp(nullptr, nullptr);
    temp.m_borderLeft = true;
    EXPECT_TRUE(true == temp.borderLeft());
}

TEST(Tab, set_border_left)
{
    Tab temp(nullptr, nullptr);
    temp.setBorderLeft(false);
    EXPECT_TRUE(false == temp.borderLeft());
}

TEST(Tab, get_display_name_by_url)
{
    Tab temp(nullptr, nullptr);

    auto expectValue1 = temp.getDisplayNameByUrl(DUrl("computer:///"));
    auto expectValue2 = temp.getDisplayNameByUrl(DUrl("file:///home/lee/Desktop"));
    auto expectValue3 = temp.getDisplayNameByUrl(DUrl());
    auto expectValue4 = temp.getDisplayNameByUrl(DUrl("recent:///"));
    auto expectValue5 = temp.getDisplayNameByUrl(DUrl("trash:///"));
    //可能有与没有初始化一些desktopfile项导致直接拿到的是默认值
//    if ("zh_CN" == QLocale::system().name()) {
//        EXPECT_TRUE("计算机" == expectValue1);
//        EXPECT_TRUE("桌面" == expectValue2);
//        EXPECT_TRUE("" == expectValue3);
//        EXPECT_TRUE("最近使用" == expectValue4);
//        EXPECT_TRUE("回收站" == expectValue5);
//    }
    EXPECT_TRUE("Computer" == expectValue1);
    EXPECT_TRUE("Desktop" == expectValue2);
    EXPECT_TRUE("" == expectValue3);
    EXPECT_TRUE("Recent" == expectValue4);
    EXPECT_TRUE("Trash" == expectValue5);
}

TEST(Tab, bounding_rect)
{
    Tab temp(nullptr, nullptr);
    temp.setFixedSize(QSize(20, 20));
    EXPECT_TRUE(QRectF(0, 0, temp.m_width, temp.m_height) == temp.boundingRect());
}

TEST(Tab, test_shape)
{
    Tab temp(nullptr, nullptr);
    temp.setFixedSize(QSize(20, 20));
    EXPECT_TRUE(QRect(0, 0, 20, 20) == temp.shape().boundingRect());
}

TEST(Tab, test_paint)
{
    QPainter testPainter;
    Stub stub;
    void (*ut_inThere)(void *obj, int a) = [](void *obj,int a){
        Q_UNUSED(obj)
        EXPECT_TRUE(1 == a);
    };
    stub.set(ADDR(QPen, setWidth), ut_inThere);

    Tab temp(nullptr, nullptr);
    temp.setFixedSize(QSize(20, 20));
    temp.setChecked(true);

    temp.paint(&testPainter, nullptr, nullptr);

    //checked : true
    Tab tempChecked(nullptr, nullptr);
    tempChecked.setFixedSize(QSize(20, 20));
    tempChecked.setChecked(true);
    tempChecked.paint(&testPainter, nullptr, nullptr);

    //checked : false
    //m_hovered: true
    Tab tempCkAndHvd(nullptr, nullptr);
    tempCkAndHvd.setFixedSize(QSize(20, 20));
    tempCkAndHvd.setChecked(false);
    tempCkAndHvd.setHovered(true);
    tempCkAndHvd.paint(&testPainter, nullptr, nullptr);

    //checked : false
    //m_hovered: false
    Tab tempCkAndHvdf(nullptr, nullptr);
    tempCkAndHvdf.setFixedSize(QSize(20, 20));
    tempCkAndHvdf.setChecked(false);
    tempCkAndHvdf.setHovered(false);
    tempCkAndHvdf.paint(&testPainter, nullptr, nullptr);
}

TEST(Tab, on_file_root_url_changed)
{
    Tab temp(nullptr, nullptr);
    temp.onFileRootUrlChanged(DUrl("/home/utTest"));
    EXPECT_TRUE(DUrl("/home/utTest") == temp.m_url);
}

TEST(Tab, mouse_release_event)
{
    Tab temp(nullptr, nullptr);
    QGraphicsSceneMouseEvent me(QEvent::MouseButtonRelease);
    temp.mouseReleaseEvent(&me);
    EXPECT_FALSE(temp.m_pressed);
    temp.m_dragOutSide = true;
    temp.mouseReleaseEvent(&me);
    EXPECT_FALSE(temp.m_dragOutSide);
}

TEST(Tab, mouse_press_event)
{
    Tab temp(nullptr, nullptr);
    QGraphicsSceneMouseEvent me(QEvent::MouseButtonPress);
    me.setButton(Qt::LeftButton);
    temp.mousePressEvent(&me);
    EXPECT_TRUE(temp.m_pressed);
}

TEST(Tab, mouse_move_event)
{
   //只考虑极限覆盖尽量考虑全量覆盖的情况

    //m_isDragging = true && m_borderLeft = false
    //(event->pos().y() < -m_height || event->pos().y() > m_height * 2) = true
    //m_dragOutSide = true
    Tab tempOne(nullptr, nullptr);
    QGraphicsSceneMouseEvent meOne(QEvent::MouseMove);
    tempOne.setFixedSize(QSize(20, 20));
    meOne.setPos(QPointF(20, 50));
    tempOne.m_isDragging = true;
    tempOne.m_borderLeft = false;
    tempOne.m_dragOutSide = true;
    tempOne.mouseMoveEvent(&meOne);
    EXPECT_TRUE(tempOne.m_borderLeft);

    //m_isDragging = true && m_borderLeft = false
    //(event->pos().y() < -m_height || event->pos().y() > m_height * 2) = false
    //m_dragOutSide = false
    //(pos().x() == 0 && pos().x() == scene()->width() - m_width) = true
    QGraphicsScene testSceneTwo;
    Tab tempTwo(nullptr, nullptr);
    testSceneTwo.addItem(&tempTwo);
    QGraphicsSceneMouseEvent meTwo(QEvent::MouseMove);
    tempTwo.setFixedSize(QSize(20, 20));
    meTwo.setPos(QPointF(0, 30));
    tempTwo.m_isDragging = true;
    tempTwo.m_borderLeft = false;
    tempTwo.m_dragOutSide = false;
    tempTwo.mouseMoveEvent(&meTwo);
    EXPECT_TRUE(tempTwo.m_borderLeft);
    EXPECT_FALSE(tempTwo.m_dragOutSide);

    //m_isDragging = true && m_borderLeft = false
    //(event->pos().y() < -m_height || event->pos().y() > m_height * 2) = false
    //m_dragOutSide = false
    //(pos().x() == 0 && pos().x() == scene()->width() - m_width) = true
    //(pos().x() < 0) = true
    QGraphicsScene testSceneThree;
    Tab tempThree(nullptr, nullptr);
    tempThree.m_originPos = QPointF(-20, 1);
    testSceneThree.addItem(&tempThree);
    QGraphicsSceneMouseEvent meThree(QEvent::MouseMove);
    tempThree.setFixedSize(QSize(20, 20));
    meThree.setPos(QPointF(-10, 30));
    tempThree.m_isDragging = true;
    tempThree.m_borderLeft = false;
    tempThree.m_dragOutSide = false;
    tempThree.mouseMoveEvent(&meThree);
    EXPECT_TRUE(tempThree.m_borderLeft);
    EXPECT_FALSE(tempThree.m_dragOutSide);

    //m_isDragging = true && m_borderLeft = false
    //(event->pos().y() < -m_height || event->pos().y() > m_height * 2) = false
    //m_dragOutSide = false
    //(pos().x() == 0 && pos().x() == scene()->width() - m_width) = false
    //(pos().x() > scene()->width() - m_width) = true
    QGraphicsScene testSceneFour;
    Tab tempFour(nullptr, nullptr);
    tempThree.m_originPos = QPointF(20, 10);
    testSceneFour.addItem(&tempFour);
    QGraphicsSceneMouseEvent meFour(QEvent::MouseMove);
    tempFour.setFixedSize(QSize(20, 20));
    meFour.setPos(QPointF(5, 30));
    tempFour.m_isDragging = true;
    tempFour.m_borderLeft = false;
    tempFour.m_dragOutSide = false;
    tempFour.mouseMoveEvent(&meFour);
    EXPECT_TRUE(tempThree.m_borderLeft);
    EXPECT_FALSE(tempThree.m_dragOutSide);
}

TEST(Tab, hover_enter_event)
{
    Tab temp(nullptr, nullptr);
    QGraphicsSceneHoverEvent se(QEvent::None);
    temp.hoverEnterEvent(&se);
    EXPECT_TRUE(temp.m_hovered);
}

TEST(Tab, hover_leave_event)
{
    Tab temp(nullptr, nullptr);
    QGraphicsSceneHoverEvent se(QEvent::None);
    temp.hoverLeaveEvent(&se);
    EXPECT_FALSE(temp.m_hovered);
    EXPECT_FALSE(temp.m_pressed);
}

TEST(TabCloseButton, tab_close_button_constructor)
{
    TabCloseButton *temp = new TabCloseButton();
    ASSERT_TRUE(nullptr != temp);
    delete temp;
    temp = nullptr;
}

TEST(TabCloseButton, bounding_rect)
{
    TabCloseButton temp;
    EXPECT_TRUE(QRectF(0, 0, 24, 24) == temp.boundingRect());
}

//TEST(TabCloseButton, test_paint)
//{
//    TabCloseButton temp;
//    temp.m_mousePressed = true;
//    temp.m_mouseHovered = true;
//    temp.show();
//    temp.hide();
//}

TEST(TabCloseButton, closing_index)
{
    TabCloseButton temp;
    temp.m_closingIndex = 1;
    EXPECT_TRUE(1 == temp.closingIndex());
}

TEST(TabCloseButton, set_closing_index)
{
    TabCloseButton temp;
    temp.setClosingIndex(1);
    EXPECT_TRUE(1 == temp.closingIndex());
}

TEST(TabCloseButton, set_active_width_tab)
{
    TabCloseButton temp;
    temp.setActiveWidthTab(true);
    EXPECT_TRUE(temp.m_activeWidthTab);
}

TEST(TabCloseButton, hover_enter_event)
{
    TabCloseButton temp;
    QGraphicsSceneHoverEvent se(QEvent::None);
    temp.hoverEnterEvent(&se);
    EXPECT_TRUE(temp.m_mouseHovered);
}

TEST(TabCloseButton, mouse_press_event)
{
    TabCloseButton temp;
    temp.m_mouseHovered = true;
    QGraphicsSceneMouseEvent me(QEvent::MouseButtonPress);
    temp.mousePressEvent(&me);
    EXPECT_FALSE(temp.m_mouseHovered);
}

TEST(TabCloseButton, mouse_release_event)
{
    TabCloseButton temp;
    QGraphicsSceneMouseEvent me(QEvent::MouseButtonRelease);
    temp.mouseReleaseEvent(&me);
    EXPECT_FALSE(temp.m_mousePressed);
}

TEST(TabCloseButton, hover_move_event)
{
    TabCloseButton temp;
    temp.m_mouseHovered = false;
    QGraphicsSceneHoverEvent se(QEvent::None);
    temp.hoverMoveEvent(&se);
    EXPECT_TRUE(temp.m_mouseHovered);
}

TEST(TabCloseButton, hover_leave_event)
{
    TabCloseButton temp;
    QGraphicsSceneHoverEvent se(QEvent::None);
    temp.hoverLeaveEvent(&se);
    EXPECT_FALSE(temp.m_mouseHovered);
}

TEST(TabBar, tab_bar_constructor)
{
    TabBar *temp = new TabBar();
    ASSERT_TRUE(nullptr != temp);
    delete temp;
    temp = nullptr;
}

//TEST(TabBar, init_connections)
//{

//}

TEST(TabBar, create_tab)
{
    TabBar temp;
    temp.hide();
    temp.createTab(nullptr);
    temp.createTab(nullptr);
    EXPECT_TRUE(1 == temp.m_currentIndex);
    temp.hide();
}

TEST(TabBar, remove_tab)
{
    //removeTab中在if(index < count)为false时会有问题，此时在获取前面tab时已经崩溃了
    TabBar temp;
    temp.hide();
    temp.createTab(nullptr);
    temp.createTab(nullptr);
    temp.removeTab(1);
    EXPECT_FALSE(temp.m_lastAddTabState);
    temp.createTab(nullptr);
    temp.createTab(nullptr);
    temp.createTab(nullptr);
    temp.createTab(nullptr);
    temp.createTab(nullptr);
    temp.createTab(nullptr);
    temp.removeTab(1);
    EXPECT_FALSE(temp.m_lastAddTabState);
    temp.m_TabCloseButton->setClosingIndex(5);
    // 绕开 new QMouseEvent 内存泄露事件
    // 据官方文档描述，要主动构造 event，必须在堆上构建,当事件循环系统派发完成后,会自动删除, 所以现有的泄露事件实属误报. 主流程中并不会造成内存泄露.
    temp.removeTab(1, false);
    EXPECT_FALSE(temp.m_lastAddTabState);
    temp.hide();
}

TEST(TabBar, set_tabText)
{
    //逻辑里面第一个和最后一个始终设置不到值好像
    TabBar temp;
    temp.hide();
    temp.createTab(nullptr);
    temp.createTab(nullptr);
    temp.setTabText(1,"tab1");
    EXPECT_TRUE("tab1" == temp.m_tabs.at(1)->tabText());
}

TEST(TabBar, count)
{
    TabBar temp;
    temp.hide();
    temp.createTab(nullptr);
    temp.createTab(nullptr);
    EXPECT_TRUE(2 == temp.count());
}

TEST(TabBar, current_index)
{
    TabBar temp;
    temp.hide();
    EXPECT_TRUE(-1 == temp.currentIndex());
    temp.m_currentIndex = 2;
    EXPECT_TRUE(2 == temp.currentIndex());
}

TEST(TabBar, tab_addable)
{
    TabBar temp;
    temp.hide();
    temp.createTab(nullptr);
    temp.createTab(nullptr);
    EXPECT_TRUE(temp.tabAddable());
    temp.createTab(nullptr);
    temp.createTab(nullptr);
    temp.createTab(nullptr);
    temp.createTab(nullptr);
    temp.createTab(nullptr);
    temp.createTab(nullptr);
    EXPECT_FALSE(temp.tabAddable());
}

TEST(TabBar, current_tab)
{
    TabBar temp;
    temp.hide();
    temp.createTab(nullptr);
    temp.m_tabs.first()->setTabText("tab1");
    temp.createTab(nullptr);
    temp.m_tabs.last()->setTabText("tab2");
    temp.setCurrentIndex(0);
    auto tp1 = temp.currentTab()->tabText();
    EXPECT_TRUE("tab1" == temp.currentTab()->tabText());
    temp.setCurrentIndex(1);
    EXPECT_TRUE("tab2" == temp.currentTab()->tabText());

}

TEST(TabBar, tab_at)
{
    TabBar temp;
    temp.hide();
    temp.createTab(nullptr);
    temp.m_tabs.first()->setTabText("tab1");
    temp.createTab(nullptr);
    temp.m_tabs.last()->setTabText("tab2");
    EXPECT_TRUE("tab1" == temp.tabAt(0)->tabText());
    EXPECT_TRUE("tab2" == temp.tabAt(1)->tabText());
}

TEST(TabBar, set_current_index)
{
    TabBar temp;
    temp.hide();
    temp.setCurrentIndex(1);
    EXPECT_TRUE(-1 == temp.m_currentIndex);
    temp.createTab(nullptr);
    temp.createTab(nullptr);
    temp.setCurrentIndex(1);
    EXPECT_TRUE(1 == temp.m_currentIndex);
}

TEST(TabBar, on_tab_close_button_hovered)
{
    TabBar temp;
    temp.hide();
    temp.createTab(nullptr);
    temp.m_tabs.first()->setTabText("tab1");
    temp.createTab(nullptr);
    temp.m_tabs.last()->setTabText("tab2");
    temp.m_tabs.last()->m_hovered = false;
    temp.onTabCloseButtonHovered(5);
    EXPECT_FALSE(temp.m_tabs.last()->m_hovered);
    temp.onTabCloseButtonHovered(1);
    EXPECT_TRUE(temp.m_tabs.last()->m_hovered);
    Tab *tpTb = nullptr;
    temp.m_tabs.append(tpTb);
    temp.onTabCloseButtonHovered(2);
}

TEST(TabBar, on_tab_close_button_unhovered)
{
    TabBar temp;
    temp.hide();
    temp.createTab(nullptr);
    temp.m_tabs.first()->setTabText("tab1");
    temp.createTab(nullptr);
    temp.m_tabs.last()->setTabText("tab2");
    temp.m_tabs.last()->m_hovered = true;
    temp.onTabCloseButtonUnHovered(5);
    EXPECT_TRUE(temp.m_tabs.last()->m_hovered);
    temp.onTabCloseButtonUnHovered(1);
    EXPECT_FALSE(temp.m_tabs.last()->m_hovered);
}

TEST(TabBar, on_move_next)
{
    TabBar temp;
    temp.hide();
    temp.createTab(nullptr);
    temp.m_tabs.first()->setTabText("tab1");
    temp.createTab(nullptr);
    temp.m_tabs.last()->setTabText("tab2");
    temp.createTab(nullptr);
    temp.m_tabs.last()->setTabText("tab3");
    auto tpTbOne = temp.tabAt(0);
    temp.onMoveNext(tpTbOne);
    EXPECT_TRUE(1 == temp.currentIndex());
    EXPECT_TRUE("tab2" == temp.tabAt(0)->tabText());
    tpTbOne = temp.tabAt(2);
    temp.onMoveNext(tpTbOne);
    EXPECT_TRUE(1 == temp.currentIndex());
    EXPECT_TRUE("tab2" == temp.tabAt(0)->tabText());
}

TEST(TabBar, on_move_previus)
{
    //感觉该函数有问题
    TabBar temp;
    temp.hide();
    temp.createTab(nullptr);
    temp.m_tabs.first()->setTabText("tab1");
    temp.createTab(nullptr);
    temp.m_tabs.last()->setTabText("tab2");
    temp.createTab(nullptr);
    temp.m_tabs.last()->setTabText("tab3");
    auto tpTbOne = temp.tabAt(1);
    temp.onMovePrevius(tpTbOne);
    EXPECT_TRUE(0 == temp.currentIndex());
    EXPECT_TRUE("tab2" == temp.tabAt(0)->tabText());
    tpTbOne = temp.tabAt(0);
    temp.onMovePrevius(tpTbOne);
    EXPECT_TRUE(0 == temp.currentIndex());
    EXPECT_TRUE("tab2" == temp.tabAt(0)->tabText());
}

//TEST(TabBar, on_request_new_window)
//{
//    TabBar temp;
//    temp.hide();
//    //会拉起一个新的文管窗口，代码层面不好关掉
////    temp.onRequestNewWindow(DUrl("file:///home/lee/Pictures"));
//}

//TEST(TabBar, on_about_to_new_window)
//{
//    TabBar temp;
//    temp.hide();
//    temp.onAboutToNewWindow(nullptr);
//}

TEST(TabBar, on_tab_clicked)
{
    TabBar temp;
    temp.hide();
    temp.createTab(nullptr);
    temp.createTab(nullptr);
    temp.m_currentIndex = 0;
    auto tb = temp.tabAt(1);
    QGraphicsSceneMouseEvent se(QEvent::MouseButtonRelease);
    tb->mouseReleaseEvent(&se);
    EXPECT_TRUE(1 == temp.m_currentIndex);
}

TEST(TabBar, on_tab_drag_finished)
{
    TabBar temp;
    temp.hide();
    temp.createTab(nullptr);
    temp.createTab(nullptr);
    temp.m_currentIndex = 0;
    auto tb = temp.tabAt(1);
    tb->m_dragOutSide = false;
    QGraphicsSceneMouseEvent se(QEvent::MouseButtonRelease);
    tb->mouseReleaseEvent(&se);
    EXPECT_FALSE(temp.m_lastDeleteState);
}

TEST(TabBar, on_tab_drag_started)
{
    TabBar temp;
    temp.hide();
    temp.createTab(nullptr);
    temp.createTab(nullptr);
    temp.m_currentIndex = 0;
    auto tb = temp.tabAt(1);
    tb->setFixedSize(QSize(20, 20));
    tb->m_isDragging = true;
    tb->m_borderLeft = false;
    tb->m_dragOutSide = false;
    QGraphicsSceneMouseEvent se(QEvent::MouseButtonRelease);
    se.setPos(QPointF(0, 30));
    tb->mouseMoveEvent(&se);
    EXPECT_TRUE(tb->m_isDragging);
}

TEST(TabBar, activate_next_tab)
{
    TabBar temp;
    temp.hide();
    temp.createTab(nullptr);
    temp.activateNextTab();
    EXPECT_TRUE(0 == temp.m_currentIndex);
    temp.createTab(nullptr);
    temp.activateNextTab();
    EXPECT_TRUE(0 == temp.m_currentIndex);
}

TEST(TabBar, activate_previous_tab)
{
    TabBar temp;
    temp.hide();
    temp.createTab(nullptr);
    temp.activatePreviousTab();
    EXPECT_TRUE(0 == temp.m_currentIndex);
    temp.createTab(nullptr);
    temp.activatePreviousTab();
    EXPECT_TRUE(0 == temp.m_currentIndex);
}
//#include "views/dfilemanagerwindow.h"
//TEST(TabBar, on_current_url_changed)
//{
//    DFileManagerWindow tpWd;
//    TabBar temp;
//    temp.hide();
//    temp.onCurrentUrlChanged(DFMUrlBaseEvent(&tpWd, DUrl("/home/utTest")));
//}

TEST(TabBar, resize_event)
{
    TabBar temp;
    temp.hide();
    auto evt = QResizeEvent(QSize(200, 20), QSize(250, 20));
    temp.resizeEvent(&evt);
}

TEST(TabBar, event)
{
    TabBar temp;
    temp.hide();
    temp.m_lastDeleteState = true;
    auto evt = QEvent(QEvent::Type::Leave);
    temp.event(&evt);
    EXPECT_FALSE(temp.m_lastDeleteState);
}

//TEST(TabBar, mouse_move_event)
//{
//    TabBar temp;
//    temp.hide();
//    temp.createTab(nullptr);
//    temp.tabAt(0)->setFixedSize(QSize(20, 50));
//    temp.createTab(nullptr);
//    QMouseEvent met(QEvent::MouseMove
//                    , QPointF(200, 30)
//                    ,Qt::MouseButton::LeftButton
//                    ,Qt::MouseButton::LeftButton
//                    ,Qt::KeyboardModifier::NoModifier);
//    temp.mouseMoveEvent(&met);
//}
