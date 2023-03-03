// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
