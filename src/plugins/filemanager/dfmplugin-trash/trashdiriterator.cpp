// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "trashdiriterator.h"
#include "utils/trashhelper.h"
#include "private/trashdiriterator_p.h"

#include <dfm-base/base/schemefactory.h>
#include <dfm-base/base/standardpaths.h>
#include <dfm-base/base/device/deviceutils.h>
#include <dfm-base/utils/universalutils.h>

DFMBASE_USE_NAMESPACE
using namespace dfmplugin_trash;

TrashDirIteratorPrivate::TrashDirIteratorPrivate(const QUrl &url, const QStringList &nameFilters,
                                                 DFMIO::DEnumerator::DirFilters filters, DFMIO::DEnumerator::IteratorFlags flags,
                                                 TrashDirIterator *qq)
    : q(qq)
{
    fstabMap = DeviceUtils::fstabBindInfo();
    dEnumerator.reset(new DFMIO::DEnumerator(url, nameFilters, filters, flags));
}

TrashDirIteratorPrivate::~TrashDirIteratorPrivate()
{
}

TrashDirIterator::TrashDirIterator(const QUrl &url,
                                   const QStringList &nameFilters,
                                   QDir::Filters filters,
                                   QDirIterator::IteratorFlags flags)
    : AbstractDirIterator(url, nameFilters, filters, flags),
      d(new TrashDirIteratorPrivate(url, nameFilters, static_cast<DFMIO::DEnumerator::DirFilter>(static_cast<int32_t>(filters)),
                                    static_cast<DFMIO::DEnumerator::IteratorFlag>(static_cast<uint8_t>(flags)), this))
{
}

TrashDirIterator::~TrashDirIterator()
{
}

QUrl TrashDirIterator::next()
{
    if (d->dEnumerator)
        d->currentUrl = d->dEnumerator->next();

    return d->currentUrl;
}

bool TrashDirIterator::hasNext() const
{
    bool has = false;
    if (d->dEnumerator)
        has = d->dEnumerator->hasNext();

    if (!has)
        return has;

    if (d->dEnumerator) {
        if (!d->once &&  UniversalUtils::urlEquals(d->dEnumerator->uri(),
                                                   TrashHelper::instance()->rootUrl()))
            TrashHelper::instance()->trashNotEmpty();

        d->once = true;
        const QUrl &urlNext = d->dEnumerator->next();
        d->fileInfo = InfoFactory::create<FileInfo>(urlNext);
        if (d->fileInfo) {
            const QUrl &urlTarget = d->fileInfo->urlOf(UrlInfoType::kRedirectedFileUrl);
            for (const QString &key : d->fstabMap.keys()) {
                if (urlTarget.path().startsWith(key))
                    return hasNext();
            }
        }
    }

    return has;
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
    if (d->dEnumerator)
        return d->dEnumerator->uri();
    return TrashHelper::rootUrl();
}
