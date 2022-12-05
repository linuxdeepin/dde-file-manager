/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     yanghao<yanghao@uniontech.com>
 *
 * Maintainer: liuyangming<liuyangming@uniontech.com>
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

#include "trashdiriterator.h"
#include "utils/trashhelper.h"
#include "private/trashdiriterator_p.h"

#include "dfm-base/base/schemefactory.h"
#include "dfm-base/base/standardpaths.h"
#include "dfm-base/base/device/deviceutils.h"
#include "dfm-base/utils/decorator/decoratorfileenumerator.h"

DFMBASE_USE_NAMESPACE
using namespace dfmplugin_trash;

TrashDirIteratorPrivate::TrashDirIteratorPrivate(const QUrl &url, const QStringList &nameFilters,
                                                 DFMIO::DEnumerator::DirFilters filters, DFMIO::DEnumerator::IteratorFlags flags,
                                                 TrashDirIterator *qq)
    : q(qq)
{
    fstabMap = DeviceUtils::fstabBindInfo();
    DecoratorFileEnumerator enumerator(url, nameFilters, filters, flags);

    dEnumerator = enumerator.enumeratorPtr();
    if (!dEnumerator) {
        qWarning("Failed dfm-io use factory create enumerator");
    }
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

    if (d->fstabMap.isEmpty())
        return has;

    if (d->dEnumerator) {
        const QUrl &urlNext = d->dEnumerator->next();
        auto fileinfo = InfoFactory::create<AbstractFileInfo>(urlNext);
        if (fileinfo) {
            const QUrl &urlTarget = fileinfo->redirectedFileUrl();
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
        return fileinfo->displayInfo(AbstractFileInfo::DisplayInfoType::kFileDisplayName);
    } else {
        return QString();
    }
}

QUrl TrashDirIterator::fileUrl() const
{
    auto fileinfo = fileInfo();
    if (fileinfo) {
        return fileinfo->redirectedFileUrl();
    } else {
        return QUrl();
    }
}

const AbstractFileInfoPointer TrashDirIterator::fileInfo() const
{
    return InfoFactory::create<AbstractFileInfo>(d->currentUrl);
}

QUrl TrashDirIterator::url() const
{
    return TrashHelper::rootUrl();
}
