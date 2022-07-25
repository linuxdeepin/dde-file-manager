/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     xushitong<xushitong@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             zhangsheng<zhangsheng@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "smbshareiterator.h"
#include "private/smbshareiterator_p.h"
#include "utils/smbbrowserutils.h"

#include <dfm-io/local/dlocalenumerator.h>

using namespace dfmplugin_smbbrowser;
DFMBASE_USE_NAMESPACE
USING_IO_NAMESPACE

// TODO(xust) TODO(lanxs): using local enumerator temperarily, using SmbBrowserEnumerator or something later.

SmbShareIteratorPrivate::SmbShareIteratorPrivate(const QUrl &url, dfmplugin_smbbrowser::SmbShareIterator *qq)
    : q(qq)
{
    {
        QMutexLocker locker(&SmbBrowserUtils::mutex);
        SmbBrowserUtils::shareNodes.clear();
    }
    enumerator.reset(new DLocalEnumerator(url));
}

SmbShareIteratorPrivate::~SmbShareIteratorPrivate()
{
}

SmbShareIterator::SmbShareIterator(const QUrl &url, const QStringList &nameFilters, QDir::Filters filters, QDirIterator::IteratorFlags flags)
    : AbstractDirIterator(url, nameFilters, filters, flags), d(new SmbShareIteratorPrivate(url, this))
{
}

SmbShareIterator::~SmbShareIterator()
{
}

QUrl SmbShareIterator::next()
{
    d->enumerator->next();
    auto info = d->enumerator->fileInfo();
    // TODO(xust) TODO(lanxs) if url contains '#', wrong info is returned
    QUrl url = info->attribute(DFileInfo::AttributeID::kStandardTargetUri).toUrl();
    QStringList icons = info->attribute(DFileInfo::AttributeID::kStandardIcon).toStringList();
    QString icon = icons.count() > 0 ? icons.first() : "folder-remote";
    QString name = info->attribute(DFileInfo::AttributeID::kStandardDisplayName).toString();

    {
        QMutexLocker locker(&SmbBrowserUtils::mutex);
        SmbShareNode node;
        node.url = url.toString();
        node.iconType = icon;
        node.displayName = name;
        SmbBrowserUtils::shareNodes.insert(url, node);
    }

    return url;
}

bool SmbShareIterator::hasNext() const
{
    return d->enumerator->hasNext();
}

QString SmbShareIterator::fileName() const
{
    return {};
}

QUrl SmbShareIterator::fileUrl() const
{
    return {};
}

const AbstractFileInfoPointer SmbShareIterator::fileInfo() const
{
    return nullptr;
}

QUrl SmbShareIterator::url() const
{
    return {};
}
