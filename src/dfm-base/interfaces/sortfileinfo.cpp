// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <dfm-base/interfaces/private/sortfileinfo_p.h>
#include <dfm-base/mimetype/mimetypedisplaymanager.h>

#include <QtConcurrent>
#include <QMutexLocker>
#include <sys/stat.h>

namespace dfmbase {

SortFileInfo::SortFileInfo()
    : d(new SortFileInfoPrivate(this))
{
}

SortFileInfo::~SortFileInfo()
{
}

void SortFileInfo::setUrl(const QUrl &url)
{
    d->url = url;
}

void SortFileInfo::setSize(const qint64 size)
{
    d->filesize = size;
}

void SortFileInfo::setFile(const bool isfile)
{
    d->file = isfile;
}

void SortFileInfo::setDir(const bool isdir)
{
    d->dir = isdir;
}

void SortFileInfo::setSymlink(const bool isSymlink)
{
    d->symLink = isSymlink;
}

void SortFileInfo::setHide(const bool ishide)
{
    d->hide = ishide;
}

void SortFileInfo::setReadable(const bool readable)
{
    d->readable = readable;
}

void SortFileInfo::setWriteable(const bool writeable)
{
    d->writeable = writeable;
}

void SortFileInfo::setExecutable(const bool executable)
{
    d->executable = executable;
}

void SortFileInfo::setLastReadTime(const qint64 time)
{
    d->lastRead = time;
}

void SortFileInfo::setLastModifiedTime(const qint64 time)
{
    d->lastModifed = time;
}

void SortFileInfo::setCreateTime(const qint64 time)
{
    d->create = time;
}

void SortFileInfo::setHighlightContent(const QString &content)
{
    d->highlightContent = content;
}

void SortFileInfo::setCustomData(const QString &key, const QVariant &value)
{
    d->customData.insert(key, value);
}

QString SortFileInfo::highlightContent() const
{
    return d->highlightContent;
}

QVariant SortFileInfo::customData(const QString &key) const
{
    return d->customData.value(key);
}

QUrl SortFileInfo::fileUrl() const
{
    return d->url;
}

qint64 SortFileInfo::fileSize() const
{
    return d->filesize;
}

bool SortFileInfo::isFile() const
{
    return d->file;
}

bool SortFileInfo::isDir() const
{
    return d->dir;
}

bool SortFileInfo::isSymLink() const
{
    return d->symLink;
}

bool SortFileInfo::isHide() const
{
    return d->hide;
}

bool SortFileInfo::isReadable() const
{
    return d->readable;
}

bool SortFileInfo::isWriteable() const
{
    return d->writeable;
}

bool SortFileInfo::isExecutable() const
{
    return d->executable;
}

qint64 SortFileInfo::lastReadTime() const
{
    return d->lastRead;
}

qint64 SortFileInfo::lastModifiedTime() const
{
    return d->lastModifed;
}

qint64 SortFileInfo::createTime() const
{
    return d->create;
}

// 信息完整性相关方法
void SortFileInfo::setInfoCompleted(const bool completed)
{
    QMutexLocker locker(&d->mutex);
    d->infoCompleted = completed;
}

void SortFileInfo::markAsCompleted()
{
    setInfoCompleted(true);
}

bool SortFileInfo::isInfoCompleted() const
{
    QMutexLocker locker(&d->mutex);
    return d->infoCompleted;
}

bool SortFileInfo::needsCompletion() const
{
    return !isInfoCompleted();
}

SortFileInfoPrivate::SortFileInfoPrivate(SortFileInfo *qq)
    : q(qq)
{
}

SortFileInfoPrivate::~SortFileInfoPrivate()
{
}

}
