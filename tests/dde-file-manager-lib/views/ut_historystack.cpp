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

#include <qdebug.h>
#include <stub.h>

#include "dfileservices.h"
#define private public
#include "views/historystack.h"

TEST(HistoryStackTest,init)
{
    HistoryStack h(10);
    EXPECT_EQ(10,h.m_threshold);
    EXPECT_EQ(-1,h.m_index);
    EXPECT_EQ(0,h.m_list.size());
}

TEST(HistoryStackTest,set_threshold)
{
    HistoryStack h(10);
    h.setThreshold(5);
    EXPECT_EQ(5,h.m_threshold);
}

TEST(HistoryStackTest,current_index)
{
    HistoryStack h(10);
    EXPECT_EQ(h.m_index,h.currentIndex());
}

TEST(HistoryStackTest,size)
{
    HistoryStack h(10);
    EXPECT_EQ(h.m_list.size(),h.size());
    h.append(DUrl("file:///home/"));
    EXPECT_EQ(h.m_list.size(),h.size());
}

TEST(HistoryStackTest,append_with_repeat)
{
    HistoryStack h(10);
    h.append(DUrl("file:///home/"));
    EXPECT_EQ(1,h.size());
    h.append(DUrl("file:///home/"));
    EXPECT_EQ(1,h.size());
}

TEST(HistoryStackTest,append)
{
    HistoryStack h(10);
    h.append(DUrl("file:///home/"));
    EXPECT_EQ(1,h.size());
    h.append(DUrl("file:///home/Desktop"));
    EXPECT_EQ(2,h.size());
}

TEST(HistoryStackTest,append_out)
{
    HistoryStack h(3);
    DUrl u1("file:///home/");
    DUrl u2("file://home2/");
    DUrl u3("file:///home3/");
    DUrl u4("file:///home4/");
    h.append(u1);
    h.append(u2);
    h.append(u3);
    h.m_index = 3;
    h.append(u4);
    qDebug() << h;
    EXPECT_EQ(3,h.size());
    EXPECT_EQ(u2,h.m_list.first());
}

TEST(HistoryStackTest,is_first_last)
{
    HistoryStack h(10);
    h.append(DUrl("file:///home/"));
    h.append(DUrl("file:///usr/"));
    h.m_index = 0;
    EXPECT_TRUE(h.isFirst());
    EXPECT_FALSE(h.isLast());

    h.m_index = 1;
    EXPECT_FALSE(h.isFirst());
    EXPECT_TRUE(h.isLast());
}

TEST(HistoryStackTest,back_with_zero)
{
    HistoryStack h(10);
    h.append(DUrl("file:///home"));
    h.m_index = 0;
    auto url = h.back();
    EXPECT_EQ(DUrl(),url);
}

TEST(HistoryStackTest,back)
{
    HistoryStack h(10);
    h.append(DUrl("file:///home/"));
    h.append(DUrl("file:///usr/"));
    h.append(DUrl("network://127.0.0.1"));
    h.m_index = 1;
    auto url = h.back();
    EXPECT_EQ(DUrl("file:///home/"),url);
}

TEST(HistoryStackTest,back_net)
{
    HistoryStack h(10);
    h.append(DUrl("file:///home/"));
    h.append(DUrl("file:///usr/"));
    h.append(DUrl("network://127.0.0.1"));
    h.m_list.append(DUrl("network://127.0.0.1"));
    h.m_index = 3;
    EXPECT_EQ(4,h.m_list.size());

    Stub st;
    bool (*func)() = [](){return true;};
    bool (DFileService::*org)(const DUrl &,const bool) = &DFileService::checkGvfsMountfileBusy;
    st.set(org,func);

    h.back();
    EXPECT_EQ(3,h.m_list.size());
}


TEST(HistoryStackTest,forward_with_end)
{
    HistoryStack h(10);
    h.append(DUrl("file:///home"));
    h.m_index = 1;
    auto url = h.forward();
    EXPECT_EQ(DUrl(),url);
}

TEST(HistoryStackTest,forward)
{
    HistoryStack h(10);
    h.append(DUrl("file:///home/"));
    h.append(DUrl("file:///usr"));
    h.append(DUrl("computer://"));
    h.append(DUrl("network://127.0.0.1"));
    h.m_index = 0;
    auto url = h.forward();
    EXPECT_EQ(DUrl("file:///usr"),url);

    url = h.forward();
    EXPECT_EQ(DUrl("computer://"),url);
}

TEST(HistoryStackTest,forward_net)
{
    HistoryStack h(10);
    h.append(DUrl("file:///home/"));
    h.append(DUrl("network://127.0.0.1"));
    h.m_index = 0;

    Stub st;
    bool (*func)() = [](){return true;};
    bool (DFileService::*org)(const DUrl &,const bool) = &DFileService::checkGvfsMountfileBusy;
    st.set(org,func);

    auto url = h.forward();
    EXPECT_EQ(DUrl("network://127.0.0.1"),url);

    h.m_list.append(DUrl("network://127.0.0.1"));
    EXPECT_EQ(3, h.m_list.size());

    url = h.forward();
    EXPECT_EQ(2, h.m_list.size());
}

TEST(HistoryStackTest,remove)
{
    HistoryStack h(10);
    h.append(DUrl("file:///home/"));
    h.append(DUrl("file:///usr"));
    EXPECT_EQ(2,h.size());
    h.removeAt(0);
    EXPECT_EQ(1,h.size());
}
