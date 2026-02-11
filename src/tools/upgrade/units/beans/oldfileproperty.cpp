// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "oldfileproperty.h"

OldFileProperty::OldFileProperty(QObject *parent)
    : QObject(parent)
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

QString OldFileProperty::getTag() const
{
    return tag_name;
}

void OldFileProperty::setTag(const QString &value)
{
    tag_name = value;
}
