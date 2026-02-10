// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "oldtagproperty.h"

OldTagProperty::OldTagProperty(QObject *parent) : QObject(parent)
{

}

int OldTagProperty::getTagIndex() const
{
    return tag_index;
}

void OldTagProperty::setTagIndex(int value)
{
    tag_index = value;
}

QString OldTagProperty::getTagName() const
{
    return tag_name;
}

void OldTagProperty::setTagName(const QString &value)
{
    tag_name = value;
}

QString OldTagProperty::getTagColor() const
{
    return tag_color;
}

void OldTagProperty::setTagColor(const QString &value)
{
    tag_color = value;
}
