// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <dfm-base/interfaces/private/sortfileinfo_p.h>

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

SortFileInfoPrivate::SortFileInfoPrivate(SortFileInfo *qq)
    : q(qq)
{
}

SortFileInfoPrivate::~SortFileInfoPrivate()
{
}

}
