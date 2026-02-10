// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "tagproperty.h"

using namespace dfm_upgrade;

TagProperty::TagProperty(QObject *parent)
    : QObject(parent)
{
}

int TagProperty::getTagIndex() const
{
    return tagIndex;
}

void TagProperty::setTagIndex(int value)
{
    tagIndex = value;
}

QString TagProperty::getTagName() const
{
    return tagName;
}

void TagProperty::setTagName(const QString &value)
{
    tagName = value;
}

QString TagProperty::getTagColor() const
{
    return tagColor;
}

void TagProperty::setTagColor(const QString &value)
{
    tagColor = value;
}

int TagProperty::getAmbiguity() const
{
    return ambiguity;
}

void TagProperty::setAmbiguity(int value)
{
    ambiguity = value;
}

QString TagProperty::getFuture() const
{
    return future;
}

void TagProperty::setFuture(const QString &value)
{
    future = value;
}
