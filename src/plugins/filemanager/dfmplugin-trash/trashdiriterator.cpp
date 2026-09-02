// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "trashdiriterator.h"
#include "utils/trashhelper.h"
#include <dfm-base/utils/trashutils.h>
#include "private/trashdiriterator_p.h"

#include <dfm-base/base/schemefactory.h>
#include <dfm-base/base/standardpaths.h>
#include <dfm-base/base/device/deviceutils.h>
#include <dfm-base/utils/universalutils.h>
#include <dfm-base/utils/fileutils.h>

#include <QDir>
#include <QFileInfo>

DFMBASE_USE_NAMESPACE
using namespace dfmplugin_trash;

TrashDirIteratorPrivate::TrashDirIteratorPrivate(const QUrl &url, const QStringList &nameFilters,
                                                 QDir::Filters filters, QDirIterator::IteratorFlags flags,
                                                 TrashDirIterator *qq)
    : q(qq)
{
    fstabMap = DeviceUtils::fstabBindInfo();
    rootUrl = url;

    // Iterate local trash directories instead of using DEnumerator on trash:///
    // which would go through gvfsd and block on stale CIFS mounts.
    const auto &dirs = TrashUtils::localTrashDirs();
    for (const QString &dir : dirs) {
        if (QDir(dir).exists())
            iterators.append(new QDirIterator(dir, nameFilters, filters, flags));
    }
}

TrashDirIteratorPrivate::~TrashDirIteratorPrivate()
{
    for (auto *it : iterators)
        delete it;
}

TrashDirIterator::TrashDirIterator(const QUrl &url,
                                   const QStringList &nameFilters,
                                   QDir::Filters filters,
                                   QDirIterator::IteratorFlags flags)
    : AbstractDirIterator(url, nameFilters, filters, flags),
      d(new TrashDirIteratorPrivate(url, nameFilters, filters, flags, this))
{
}

TrashDirIterator::~TrashDirIterator()
{
}

QUrl TrashDirIterator::next()
{
    return d->currentUrl;
}

bool TrashDirIterator::hasNext() const
{
    while (d->currentIteratorIndex < d->iterators.size()) {
        QDirIterator *it = d->iterators[d->currentIteratorIndex];
        if (!it->hasNext()) {
            d->currentIteratorIndex++;
            continue;
        }

        QString localPath = it->next();
        QFileInfo fi(localPath);

        // Convert local trash file path to trash:/// URL
        QUrl trashUrl = TrashUtils::localFileToTrashUrl(localPath);
        d->currentUrl = trashUrl;

        // Create FileInfo for the trash URL
        d->fileInfo = InfoFactory::create<FileInfo>(trashUrl);
        if (d->fileInfo) {
            const QUrl &urlTarget = d->fileInfo->urlOf(UrlInfoType::kRedirectedFileUrl);
            // Skip files whose target path is on a fstab bind mount (avoid duplicates)
            for (const QString &key : d->fstabMap.keys()) {
                if (urlTarget.path().startsWith(key))
                    return hasNext();
            }
        }

        if (!d->once) {
            TrashHelper::instance()->trashNotEmpty();
            d->once = true;
        }

        return true;
    }

    return false;
}

QString TrashDirIterator::fileName() const
{
    auto fileinfo = fileInfo();
    if (fileinfo) {
        return fileinfo->displayOf(DisPlayInfoType::kFileDisplayName);
    } else {
        fmWarning() << "Trash: No file info available for file name retrieval";
        return QString();
    }
}

QUrl TrashDirIterator::fileUrl() const
{
    auto fileinfo = fileInfo();
    if (fileinfo) {
        return fileinfo->urlOf(UrlInfoType::kRedirectedFileUrl);
    } else {
        fmWarning() << "Trash: No file info available for file URL retrieval";
        return QUrl();
    }
}

const FileInfoPointer TrashDirIterator::fileInfo() const
{
    if (d->fileInfo)
        return d->fileInfo;

    return InfoFactory::create<FileInfo>(d->currentUrl, Global::CreateFileInfoType::kCreateFileInfoSync);
}

QUrl TrashDirIterator::url() const
{
    return d->rootUrl.isValid() ? d->rootUrl : TrashUtils::trashRootUrl();
}
