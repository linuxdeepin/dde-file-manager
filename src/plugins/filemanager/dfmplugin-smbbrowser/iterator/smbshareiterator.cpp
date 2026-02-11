// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "smbshareiterator.h"
#include "private/smbshareiterator_p.h"
#include "utils/smbbrowserutils.h"

using namespace dfmplugin_smbbrowser;
DFMBASE_USE_NAMESPACE
USING_IO_NAMESPACE

// TODO(xust) TODO(lanxs): using local enumerator temperarily, using SmbBrowserEnumerator or something later.

SmbShareIteratorPrivate::SmbShareIteratorPrivate(const QUrl &url, dfmplugin_smbbrowser::SmbShareIterator *qq)
    : q(qq), rootUrl(url)
{
    {
        QMutexLocker locker(&smb_browser_utils::nodesMutex());
        smb_browser_utils::shareNodes().clear();
    }
    enumerator.reset(new DEnumerator(url));
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
    if (!info)
        return {};

    // TODO(xust) TODO(lanxs) if url contains '#', wrong info is returned
    QUrl url = QUrl::fromPercentEncoding(info->attribute(DFileInfo::AttributeID::kStandardTargetUri).toString().toLocal8Bit());
    QStringList icons = info->attribute(DFileInfo::AttributeID::kStandardIcon).toStringList();
    QString icon = icons.count() > 0 ? icons.first() : "folder-remote";
    QString name = info->attribute(DFileInfo::AttributeID::kStandardDisplayName).toString();

    int serverPort = d->rootUrl.port();
    if (serverPort != -1)
        url.setPort(serverPort);

    {
        QMutexLocker locker(&smb_browser_utils::nodesMutex());
        SmbShareNode node;
        node.url = url.toString();
        node.iconType = icon;
        node.displayName = name;
        smb_browser_utils::shareNodes().insert(url, node);
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

const FileInfoPointer SmbShareIterator::fileInfo() const
{
    return nullptr;
}

QUrl SmbShareIterator::url() const
{
    return {};
}

bool SmbShareIterator::initIterator()
{
    if (d->enumerator)
        return d->enumerator->initEnumerator(oneByOne());
    return false;
}
