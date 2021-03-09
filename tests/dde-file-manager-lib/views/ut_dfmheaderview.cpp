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
#include <views/dfmheaderview.h>
#include <QTest>


class DFMHeaderViewTest : public DFMHeaderView
{
public:
    DFMHeaderViewTest(Qt::Orientation orientation, QWidget *parent):DFMHeaderView(orientation, parent) {
    }
//    QSize sizeHint() const override {
//        auto size = DFMHeaderView::sizeHint();
//        return size;
//    }
protected:
    void mouseReleaseEvent(QMouseEvent *e) override {
        DFMHeaderView::mouseReleaseEvent(e);
        m_mouseReleaseEventDone = true;
    }
    void resizeEvent(QResizeEvent *e) override {
        DFMHeaderView::resizeEvent(e);
        m_resizeEventDone = true;
    }
public:
    bool m_sizeHintDone = false;
    bool m_mouseReleaseEventDone = false;
    bool m_resizeEventDone = false;
};

TEST(DFMHeaderViewTest, mouseReleaseEvent){
    DFMHeaderViewTest headerV(Qt::Horizontal,nullptr);

    QTest::mouseRelease(&headerV, Qt::LeftButton,Qt::KeyboardModifier(),QPoint());
    QMouseEvent me(QEvent::MouseButtonRelease, QPoint(30, 30), headerV.mapToGlobal(QPoint(30, 30)), Qt::LeftButton, Qt::LeftButton, Qt::KeyboardModifiers());
    headerV.mouseReleaseEvent(&me);
    EXPECT_TRUE(headerV.m_mouseReleaseEventDone);
}

TEST(DFMHeaderViewTest, resizeEvent){
    DFMHeaderViewTest headerV(Qt::Horizontal,nullptr);

    QResizeEvent re(QSize(100, 100), QSize(200, 200));
    headerV.resizeEvent(&re);
    EXPECT_TRUE(headerV.m_resizeEventDone);
}

TEST(DFMHeaderView, sizeHint){
    QWidget wdg;
    DFMHeaderView headerV(Qt::Horizontal,&wdg);
    auto expectV = headerV.sizeHint().width() == headerV.length();
    EXPECT_TRUE(expectV);

}
