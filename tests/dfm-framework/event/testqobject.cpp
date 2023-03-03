// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "testqobject.h"

#include <QDebug>

TestQObject::TestQObject(QObject *parent)
    : QObject(parent)
{
}

TestQObject::~TestQObject()
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

void TestQObject::add1(int *val)
{
    *val = *val + 1;
}
