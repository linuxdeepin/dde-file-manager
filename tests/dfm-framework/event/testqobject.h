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
#ifndef TESTQOBJECT_H
#define TESTQOBJECT_H

#include <QObject>

class TestQObject : public QObject
{
    Q_OBJECT
public:
    explicit TestQObject(QObject *parent = nullptr);
    ~TestQObject() override;

public slots:
    int test1(int a);
    bool bigger10(int v, int *called);
    bool bigger15(int v, int *called);
    bool empty1();
    bool empty2();
    void add1(int *val);
};

Q_DECLARE_METATYPE(int *);

#endif   // TESTQOBJECT_H
