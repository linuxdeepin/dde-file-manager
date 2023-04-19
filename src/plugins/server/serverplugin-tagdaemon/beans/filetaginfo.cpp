// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "filetaginfo.h"

SERVERTAGDAEMON_BEGIN_NAMESPACE

FileTagInfo::FileTagInfo(QObject *parent)
    : QObject(parent)
{
}

int FileTagInfo::getFileIndex() const
{
    return fileIndex;
}

void FileTagInfo::setFileIndex(int value)
{
    fileIndex = value;
}

QString FileTagInfo::getFilePath() const
{
    return filePath;
}

void FileTagInfo::setFilePath(const QString &value)
{
    filePath = value;
}

QString FileTagInfo::getTagName() const
{
    return tagName;
}

void FileTagInfo::setTagName(const QString &value)
{
    tagName = value;
}

int FileTagInfo::getTagOrder() const
{
    return tagOrder;
}

void FileTagInfo::setTagOrder(int value)
{
    tagOrder = value;
}

QString FileTagInfo::getFuture() const
{
    return future;
}

void FileTagInfo::setFuture(const QString &value)
{
    future = value;
}

SERVERTAGDAEMON_END_NAMESPACE
