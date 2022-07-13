/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangyu<zhangyub@uniontech.com>
 *
 * Maintainer: zhangyu<zhangyub@uniontech.com>
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

#define private public
#include "views/dtagedit.h"

TEST(DTagEditTest,init)
{
    DTagEdit w;
    EXPECT_NE(nullptr,w.m_crumbEdit);
    EXPECT_NE(nullptr,w.m_promptLabel);
    EXPECT_NE(nullptr,w.m_totalLayout);
    EXPECT_NE(nullptr,w.m_BGFrame);
    EXPECT_EQ(false,w.m_flagForShown.load());
}

TEST(DTagEditTest,set_focusoutself_closing)
{
    DTagEdit w;
    w.setFocusOutSelfClosing(true);
    EXPECT_EQ(true,w.m_flagForShown.load());

    w.setFocusOutSelfClosing(false);
    EXPECT_EQ(false,w.m_flagForShown.load());
}

TEST(DTagEditTest,on_focusout_with_exit)
{
    DTagEdit w;
    w.show(0,0);
    EXPECT_EQ(true,w.isVisible());
    w.setFocusOutSelfClosing(true);
    w.onFocusOut();
    EXPECT_NE(true,w.isVisible());
}

TEST(DTagEditTest,on_focusout_without_exit)
{
    DTagEdit w;
    w.show(0,0);
    EXPECT_EQ(true,w.isVisible());
    w.setFocusOutSelfClosing(false);
    w.onFocusOut();
    EXPECT_EQ(true,w.isVisible());
}

TEST(DTagEditTest,set_files_for_tagging_empty)
{
    DTagEdit w;
    w.setFilesForTagging({});
    EXPECT_EQ(true,w.m_files.isEmpty());
}

TEST(DTagEditTest,set_files_for_tagging)
{
    DTagEdit w;
    QList<DUrl> url;
    url.append(DUrl("file:///home"));
    w.setFilesForTagging(url);
    EXPECT_EQ(url,w.m_files);
}

TEST(DTagEditTest,key_press_event_escape)
{
    DTagEdit w;
    w.show(0,0);
    EXPECT_EQ(true,w.isVisible());
    QTest::keyPress(&w,Qt::Key_Escape);
    QEventLoop loop;
    QTimer::singleShot(100,&loop,[&loop](){
        loop.quit();
    });
    loop.exec();
    EXPECT_EQ(false,w.isVisible());
}

TEST(DTagEditTest,key_press_event_enter)
{
    DTagEdit w;
    w.show(0,0);
    EXPECT_EQ(true,w.isVisible());
    QTest::keyPress(&w,Qt::Key_Enter);
    QEventLoop loop;
    QTimer::singleShot(100,&loop,[&loop](){
        loop.quit();
    });
    loop.exec();
    EXPECT_EQ(false,w.isVisible());
}

TEST(DTagEditTest,key_press_event_return)
{
    DTagEdit w;
    w.show(0,0);
    EXPECT_EQ(true,w.isVisible());
    QTest::keyPress(&w,Qt::Key_Return);
    QEventLoop loop;
    QTimer::singleShot(100,&loop,[&loop](){
        loop.quit();
    });
    loop.exec();
    EXPECT_EQ(false,w.isVisible());
}
