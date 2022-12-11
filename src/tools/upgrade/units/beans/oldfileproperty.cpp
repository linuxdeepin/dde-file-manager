// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "oldfileproperty.h"


OldFileProperty::OldFileProperty(QObject *parent) : QObject(parent)
{

}

QString OldFileProperty::getFilePath() const
{
    return file_name;
}

void OldFileProperty::setFilePath(const QString &value)
{
    file_name = value;
}

QString OldFileProperty::getTag1() const
{
    return tag_1;
}

void OldFileProperty::setTag1(const QString &value)
{
    tag_1 = value;
}

QString OldFileProperty::getTag2() const
{
    return tag_2;
}

void OldFileProperty::setTag2(const QString &value)
{
    tag_2 = value;
}

QString OldFileProperty::getTag3() const
{
    return tag_3;
}

void OldFileProperty::setTag3(const QString &value)
{
    tag_3 = value;
}

