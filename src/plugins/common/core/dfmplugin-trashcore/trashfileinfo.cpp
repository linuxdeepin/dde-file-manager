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
#include "trashfileinfo.h"
#include "utils/trashcorehelper.h"

#include "interfaces/private/abstractfileinfo_p.h"
#include "dfm-base/base/schemefactory.h"
#include "dfm-base/utils/fileutils.h"
#include "dfm-base/file/local/desktopfileinfo.h"
#include "dfm-base/utils/decorator/decoratorfileenumerator.h"
#include "dfm-base/utils/universalutils.h"

#include <QCoreApplication>

DFMBASE_USE_NAMESPACE
namespace dfmplugin_trashcore {
class TrashFileInfoPrivate : public AbstractFileInfoPrivate
{
public:
    explicit TrashFileInfoPrivate(AbstractFileInfo *qq)
        : AbstractFileInfoPrivate(qq)
    {
    }

    virtual ~TrashFileInfoPrivate();

    QSharedPointer<DFileInfo> dFileInfo { nullptr };
    QUrl targetUrl;
};

TrashFileInfoPrivate::~TrashFileInfoPrivate()
{
}

TrashFileInfo::TrashFileInfo(const QUrl &url)
    : AbstractFileInfo(url, new TrashFileInfoPrivate(this))
{
    d = static_cast<TrashFileInfoPrivate *>(dptr.data());

    QSharedPointer<DIOFactory> factory = produceQSharedIOFactory(url.scheme(), static_cast<QUrl>(url));
    if (!factory) {
        qWarning() << "dfm-io create factory failed, url: " << url;
        return;
    }

    d->dFileInfo = factory->createFileInfo();
    if (!d->dFileInfo) {
        qWarning() << "dfm-io use factory create fileinfo Failed, url: " << url;
        return;
    }
    bool init = d->dFileInfo->initQuerier();
    if (!init) {
        qWarning() << "querier init failed, url: " << url;
        return;
    }

    d->targetUrl = d->dFileInfo->attribute(DFileInfo::AttributeID::kStandardTargetUri).toUrl();
    if (d->targetUrl.isValid())
        setProxy(InfoFactory::create<AbstractFileInfo>(d->targetUrl));
    else
        qWarning() << "create proxy failed, target url is invalid, url: " << url;
}

TrashFileInfo::~TrashFileInfo()
{
}

QString TrashFileInfo::fileName() const
{
    if (!d->dFileInfo)
        return QString();

    if (!d->targetUrl.isValid())
        return QString();

    if (FileUtils::isDesktopFile(d->targetUrl)) {
        DesktopFileInfo dfi(d->targetUrl);
        return dfi.fileDisplayName();
    }

    return d->dFileInfo->attribute(DFileInfo::AttributeID::kStandardName).toString();
}

QString TrashFileInfo::fileDisplayName() const
{
    if (url() == TrashCoreHelper::rootUrl())
        return QCoreApplication::translate("PathManager", "Trash");

    if (!d->dFileInfo)
        return QString();

    if (!d->targetUrl.isValid())
        return QString();

    if (FileUtils::isDesktopFile(d->targetUrl)) {
        DesktopFileInfo dfi(d->targetUrl);
        return dfi.fileDisplayName();
    }

    return d->dFileInfo->attribute(DFileInfo::AttributeID::kStandardDisplayName).toString();
}

bool TrashFileInfo::exists() const
{
    return AbstractFileInfo::exists() || url() == TrashCoreHelper::rootUrl();
}

bool TrashFileInfo::canDelete() const
{
    if (!d->dFileInfo)
        return false;

    bool value = false;
    bool success = false;
    value = d->dFileInfo->attribute(DFileInfo::AttributeID::kAccessCanDelete, &success).toBool();
    return value && success;
}

bool TrashFileInfo::canTrash() const
{
    if (!d->dFileInfo)
        return false;

    bool value = false;
    bool success = false;
    value = d->dFileInfo->attribute(DFileInfo::AttributeID::kAccessCanTrash, &success).toBool();
    return value && success;
}

void TrashFileInfo::refresh()
{
    AbstractFileInfo::refresh();
}

bool TrashFileInfo::canRename() const
{
    if (!d->dFileInfo)
        return false;

    bool value = false;
    bool success = false;
    value = d->dFileInfo->attribute(DFileInfo::AttributeID::kAccessCanRename, &success).toBool();
    return value && success;
}

QFile::Permissions TrashFileInfo::permissions() const
{
    QFileDevice::Permissions p = AbstractFileInfo::permissions();

    p &= ~QFileDevice::WriteOwner;
    p &= ~QFileDevice::WriteUser;
    p &= ~QFileDevice::WriteGroup;
    p &= ~QFileDevice::WriteOther;

    return p;
}

QIcon TrashFileInfo::fileIcon()
{
    if (!d->targetUrl.isValid())
        return QIcon();

    if (FileUtils::isDesktopFile(d->targetUrl)) {
        DesktopFileInfo dfi(d->targetUrl);
        return dfi.fileIcon();
    }

    return AbstractFileInfo::fileIcon();
}

QDateTime TrashFileInfo::lastRead() const
{
    if (!d->dFileInfo)
        return QDateTime();

    QDateTime time;
    bool success = false;
    uint64_t data = d->dFileInfo->attribute(DFileInfo::AttributeID::kTimeAccess, &success).value<uint64_t>();
    if (success)
        time = QDateTime::fromTime_t(static_cast<uint>(data));
    return time;
}

QDateTime TrashFileInfo::lastModified() const
{
    if (!d->dFileInfo)
        return QDateTime();

    QDateTime time;
    bool success = false;
    uint64_t data = d->dFileInfo->attribute(DFileInfo::AttributeID::kTimeModified, &success).value<uint64_t>();
    if (success)
        time = QDateTime::fromTime_t(static_cast<uint>(data));
    return time;
}

qint64 TrashFileInfo::size() const
{
    if (!d->dFileInfo)
        return qint64();

    qint64 size = 0;
    const QUrl &fileUrl = url();
    if (fileUrl == FileUtils::trashRootUrl()) {
        DecoratorFileEnumerator enumerator(fileUrl);
        if (!enumerator.isValid())
            return qint64();
        while (enumerator.hasNext()) {
            const QUrl &urlNext = enumerator.next();
            AbstractFileInfoPointer fileInfo = InfoFactory::create<AbstractFileInfo>(urlNext);
            if (!fileInfo)
                continue;
            size += fileInfo->size();
        }
        return size;
    }

    bool success = false;
    size = d->dFileInfo->attribute(DFileInfo::AttributeID::kStandardSize, &success).value<qint64>();
    return size;
}

bool TrashFileInfo::isSymLink() const
{
    if (!d->dFileInfo)
        return false;

    bool isSymLink = false;
    bool success = false;
    isSymLink = d->dFileInfo->attribute(DFileInfo::AttributeID::kStandardIsSymlink, &success).toBool();
    return isSymLink;
}

QString TrashFileInfo::symLinkTarget() const
{
    if (!d->dFileInfo)
        return QString();

    QString symLinkTarget;
    bool success = false;
    symLinkTarget = d->dFileInfo->attribute(DFileInfo::AttributeID::kStandardSymlinkTarget, &success).toString();
    return symLinkTarget;
}

QString TrashFileInfo::mimeTypeName()
{
    if (!d->dFileInfo)
        return QString();

    QString type;
    bool success = false;
    type = d->dFileInfo->attribute(DFileInfo::AttributeID::kStandardContentType, &success).toString();
    return type;
}

int TrashFileInfo::countChildFile() const
{
    if (isDir()) {
        DecoratorFileEnumerator enumerator(url());
        return int(enumerator.fileCount());
    }

    return -1;
}

bool TrashFileInfo::isReadable() const
{
    if (!d->dFileInfo)
        return false;

    bool value = false;
    bool success = false;
    value = d->dFileInfo->attribute(DFileInfo::AttributeID::kAccessCanRead, &success).toBool();
    return value && success;
}

bool TrashFileInfo::isWritable() const
{
    if (!d->dFileInfo)
        return false;

    bool value = false;
    bool success = false;
    value = d->dFileInfo->attribute(DFileInfo::AttributeID::kAccessCanWrite, &success).toBool();
    return value && success;
}

bool TrashFileInfo::isDir() const
{
    if (url() == TrashCoreHelper::rootUrl()) {
        return true;
    }

    return AbstractFileInfo::isDir();
}

bool TrashFileInfo::canDrop()
{
    const QUrl &fileUrl = url();
    return UniversalUtils::urlEquals(fileUrl, TrashCoreHelper::rootUrl());
}

bool TrashFileInfo::canHidden() const
{
    return false;
}

QUrl TrashFileInfo::originalUrl() const
{
    if (!d->dFileInfo)
        return QUrl();

    return QUrl::fromLocalFile(d->dFileInfo->attribute(DFileInfo::AttributeID::kTrashOrigPath).toString());
}

QUrl TrashFileInfo::redirectedFileUrl() const
{
    return d->targetUrl;
}

bool TrashFileInfo::isHidden() const
{
    return false;
}

QDateTime TrashFileInfo::deletionTime() const
{
    if (!d->dFileInfo)
        return QDateTime();

    return QDateTime::fromString(d->dFileInfo->attribute(DFileInfo::AttributeID::kTrashDeletionDate).toString(), Qt::ISODate);
}

}
