/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     xinglinkun<xinglinkun@uniontech.com>
 *
 * Maintainer: xinglinkun<xinglinkun@uniontech.com>
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
#define protected public

#include <QWindow>
#include <QWidget>
#include "util/util.h"
#include "stub.h"
#include "desktopinfo.h"

QWindow* mywindowhandle()
{
    return nullptr;
}
TEST(UtilTest, test_set_desktop_window)
{
    Stub stu, stub1;
    static bool judge = false;
    DesktopUtil::set_desktop_window(nullptr);
    QWidget* widget = new QWidget;
    DesktopUtil::set_desktop_window(widget);
    bool (*mydesktopinfo)() = [](){
        judge = true;
        return true;};
    stub1.set(ADDR(DesktopInfo, waylandDectected), mydesktopinfo);
    DesktopUtil::set_desktop_window(widget);
    DesktopUtil::set_desktop_window(widget);

    stu.set(ADDR(QWidget, windowHandle), mywindowhandle);
    DesktopUtil::set_desktop_window(nullptr);
    EXPECT_TRUE(judge);
    delete widget;
}

TEST(UtilTest, test_set_prview_window)
{
    DesktopUtil::set_prview_window(nullptr);
    QWidget* widget = new QWidget;
    Stub stu,stu1;
    static bool mjudge = false;

    bool (*mywayland)() = [](){
        mjudge = true;
        return true;};
    stu1.set(ADDR(DesktopInfo, waylandDectected), mywayland);
    DesktopUtil::set_prview_window(widget);

    stu.set(ADDR(QWidget, windowHandle), mywindowhandle);
    DesktopUtil::set_prview_window(widget);
    EXPECT_TRUE(mjudge);
    delete widget;
}
