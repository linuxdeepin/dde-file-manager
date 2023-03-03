// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_testobj_user.h"

using namespace TestObj;

User::User(QObject *parent)
    : QObject(parent)
{
}

int User::getId() const
{
    return id;
}

void User::setId(int value)
{
    id = value;
}

QString User::getName() const
{
    return name;
}

void User::setName(const QString &value)
{
    name = value;
}

QString User::getPassword() const
{
    return password;
}

void User::setPassword(const QString &value)
{
    password = value;
}

QString User::getEmail() const
{
    return email;
}

void User::setEmail(const QString &value)
{
    email = value;
}

double User::getHeight() const
{
    return height;
}

void User::setHeight(double value)
{
    height = value;
}

double User::getWeight() const
{
    return weight;
}

void User::setWeight(double value)
{
    weight = value;
}
