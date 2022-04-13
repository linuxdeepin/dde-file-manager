/*
 * Copyright (C) 2021 ~ 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangsheng<zhangsheng@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             xushitong<xushitong@uniontech.com>
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
#include "testqobject.h"

#include <QDebug>

TestQObject::TestQObject(QObject *parent)
    : QObject(parent)
{
}

int TestQObject::test1(int a)
{
    return a + 10;
}

bool TestQObject::bigger10(int v, int *called)
{
    *called = 10;
    return v > 10;
}

bool TestQObject::bigger15(int v, int *called)
{
    *called = 15;
    return v > 15;
}

bool TestQObject::empty1()
{
    qDebug() << __PRETTY_FUNCTION__;
    return false;
}

bool TestQObject::empty2()
{
    qDebug() << __PRETTY_FUNCTION__;
    return true;
}
