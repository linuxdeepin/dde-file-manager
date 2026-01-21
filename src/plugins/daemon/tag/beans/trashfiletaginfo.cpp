// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "trashfiletaginfo.h"

DAEMONPTAG_BEGIN_NAMESPACE

TrashFileTagInfo::TrashFileTagInfo(QObject *parent)
    : QObject(parent)
{
}

int TrashFileTagInfo::getTrashIndex() const
{
    return trashIndex;
}

void TrashFileTagInfo::setTrashIndex(int value)
{
    trashIndex = value;
}

QString TrashFileTagInfo::getOriginalPath() const
{
    return originalPath;
}

void TrashFileTagInfo::setOriginalPath(const QString &value)
{
    originalPath = value;
}

qint64 TrashFileTagInfo::getFileInode() const
{
    return fileInode;
}

void TrashFileTagInfo::setFileInode(qint64 value)
{
    fileInode = value;
}

QString TrashFileTagInfo::getTagNames() const
{
    return tagNames;
}

void TrashFileTagInfo::setTagNames(const QString &value)
{
    tagNames = value;
}

qint64 TrashFileTagInfo::getDeleteTime() const
{
    return deleteTime;
}

void TrashFileTagInfo::setDeleteTime(qint64 value)
{
    deleteTime = value;
}

QString TrashFileTagInfo::getFuture() const
{
    return future;
}

void TrashFileTagInfo::setFuture(const QString &value)
{
    future = value;
}

DAEMONPTAG_END_NAMESPACE
