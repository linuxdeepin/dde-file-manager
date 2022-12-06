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

#include "dfm_global_defines.h"
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
    explicit TrashFileInfoPrivate(const QUrl &url, AbstractFileInfo *qq)
        : AbstractFileInfoPrivate(url, qq)
    {
    }

    virtual ~TrashFileInfoPrivate();

    QUrl initTarget();
    QString fileName() const;
    QString mimeTypeName();
    QDateTime lastRead() const;
    QDateTime lastModified() const;
    QDateTime deletionTime() const;
    QString symLinkTarget() const;

    QSharedPointer<DFileInfo> dFileInfo { nullptr };
    QSharedPointer<DFileInfo> dAncestorsFileInfo { nullptr };
    QUrl targetUrl;
    QUrl originalUrl;
    QUrl trashAncestors;
};

TrashFileInfoPrivate::~TrashFileInfoPrivate()
{
}

QUrl TrashFileInfoPrivate::initTarget()
{
    targetUrl = dFileInfo->attribute(DFileInfo::AttributeID::kStandardTargetUri).toUrl();
    originalUrl = QUrl::fromUserInput(dFileInfo->attribute(DFileInfo::AttributeID::kTrashOrigPath).toString());
    if (!targetUrl.isValid() && !UniversalUtils::urlEquals(TrashCoreHelper::rootUrl(), url)) {
        QUrl ancestors = url;
        while (TrashCoreHelper::rootUrl().isParentOf(ancestors)) {
            QUrl urlPre = ancestors;
            ancestors = UrlRoute::urlParent(ancestors);
            if (UniversalUtils::urlEquals(TrashCoreHelper::rootUrl(), ancestors)) {
                ancestors = urlPre;
                break;
            }
        }

        auto factory = produceQSharedIOFactory(ancestors.scheme(), static_cast<QUrl>(ancestors));
        auto fileinfo = factory->createFileInfo();
        if (fileinfo->initQuerier()) {
            const QUrl &ancestorsTargetUrl = fileinfo->attribute(DFileInfo::AttributeID::kStandardTargetUri).toUrl();
            if (ancestorsTargetUrl.isValid()) {
                trashAncestors = ancestors;

                QString localRootPath = ancestorsTargetUrl.toString();
                const QString &fileSuffix = url.path().mid(url.path().indexOf("/", 1));
                const QUrl &urlReal = localRootPath + fileSuffix;

                targetUrl = urlReal;
                QString localRootOriginalPath = fileinfo->attribute(DFileInfo::AttributeID::kTrashOrigPath).toString();
                originalUrl = QUrl::fromUserInput(localRootOriginalPath + fileSuffix);
                dAncestorsFileInfo = fileinfo;
                return urlReal;
            }
        }
    }
    return targetUrl;
}

QString TrashFileInfoPrivate::fileName() const
{
    if (!dFileInfo)
        return QString();

    if (targetUrl.isValid()) {
        if (FileUtils::isDesktopFile(targetUrl)) {
            DesktopFileInfo dfi(targetUrl);
            return dfi.displayInfo(AbstractFileInfo::DisplayInfoType::kFileDisplayName);
        }
    }

    return dFileInfo->attribute(DFileInfo::AttributeID::kStandardName).toString();
}

QString TrashFileInfoPrivate::mimeTypeName()
{
    if (!dFileInfo)
        return QString();

    QString type;
    bool success = false;
    type = dFileInfo->attribute(DFileInfo::AttributeID::kStandardContentType, &success).toString();
    return type;
}

QDateTime TrashFileInfoPrivate::lastRead() const
{
    if (!dFileInfo)
        return QDateTime();

    QDateTime time;
    bool success = false;
    uint64_t data = dFileInfo->attribute(DFileInfo::AttributeID::kTimeAccess, &success).value<uint64_t>();
    if (success)
        time = QDateTime::fromTime_t(static_cast<uint>(data));
    return time;
}

QDateTime TrashFileInfoPrivate::lastModified() const
{
    if (!dFileInfo)
        return QDateTime();

    QDateTime time;
    bool success = false;
    uint64_t data = dFileInfo->attribute(DFileInfo::AttributeID::kTimeModified, &success).value<uint64_t>();
    if (success)
        time = QDateTime::fromTime_t(static_cast<uint>(data));
    return time;
}

QDateTime TrashFileInfoPrivate::deletionTime() const
{
    if (dAncestorsFileInfo)
        return QDateTime::fromString(dAncestorsFileInfo->attribute(DFileInfo::AttributeID::kTrashDeletionDate).toString(), Qt::ISODate);

    if (!dFileInfo)
        return QDateTime();

    return QDateTime::fromString(dFileInfo->attribute(DFileInfo::AttributeID::kTrashDeletionDate).toString(), Qt::ISODate);
}

TrashFileInfo::TrashFileInfo(const QUrl &url)
    : AbstractFileInfo(url), d(new TrashFileInfoPrivate(url, this))
{
    dptr.reset(d);
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

    const QUrl &urlTarget = d->initTarget();
    if (urlTarget.isValid()) {
        setProxy(InfoFactory::create<AbstractFileInfo>(urlTarget));
    } else {
        if (!FileUtils::isTrashRootFile(url))
            qWarning() << "create proxy failed, target url is invalid, url: " << url;
    }
}

TrashFileInfo::~TrashFileInfo()
{
}

bool TrashFileInfo::exists() const
{
    return AbstractFileInfo::exists()
            || FileUtils::isTrashRootFile(urlInfo(AbstractFileInfo::FileUrlInfoType::kUrl));
}

bool TrashFileInfo::canDelete() const
{
    if (!d->dFileInfo)
        return false;

    bool value = false;
    bool success = false;
    value = d->dFileInfo->attribute(DFileInfo::AttributeID::kAccessCanDelete, &success).toBool();
    return value;
}

bool TrashFileInfo::canTrash() const
{
    if (!d->dFileInfo)
        return false;

    bool value = false;
    bool success = false;
    value = d->dFileInfo->attribute(DFileInfo::AttributeID::kAccessCanTrash, &success).toBool();
    return value;
}

void TrashFileInfo::refresh()
{
    AbstractFileInfo::refresh();
}

QString TrashFileInfo::nameInfo(const AbstractFileInfo::FileNameInfoType type) const
{
    switch (type) {
    case AbstractFileInfo::FileNameInfoType::kFileName:
        return d->fileName();
    case AbstractFileInfo::FileNameInfoType::kFileCopyName:
        return displayInfo(AbstractFileInfo::DisplayInfoType::kFileDisplayName);
    case AbstractFileInfo::FileNameInfoType::kMimeTypeName:
        return const_cast<TrashFileInfoPrivate *>(d)->mimeTypeName();
    default:
        return AbstractFileInfo::nameInfo(type);
    }
}

QString TrashFileInfo::displayInfo(const AbstractFileInfo::DisplayInfoType type) const
{
    if (AbstractFileInfo::DisplayInfoType::kFileDisplayName == type) {
        if (urlInfo(AbstractFileInfo::FileUrlInfoType::kUrl) == TrashCoreHelper::rootUrl())
            return QCoreApplication::translate("PathManager", "Trash");

        if (!d->dFileInfo)
            return QString();

        if (d->targetUrl.isValid()) {
            if (FileUtils::isDesktopFile(d->targetUrl)) {
                DesktopFileInfo dfi(d->targetUrl);
                return dfi.displayInfo(AbstractFileInfo::DisplayInfoType::kFileDisplayName);
            }
        }

        return d->dFileInfo->attribute(DFileInfo::AttributeID::kStandardDisplayName).toString();
    }

    return AbstractFileInfo::displayInfo(type);
}
QString TrashFileInfo::pathInfo(const dfmbase::AbstractFileInfo::FilePathInfoType type) const
{
    switch (type) {
    case FilePathInfoType::kSymLinkTarget:
        return d->symLinkTarget();
    default:
        return AbstractFileInfo::pathInfo(type);
    }
}

QUrl TrashFileInfo::urlInfo(const AbstractFileInfo::FileUrlInfoType type) const
{
    switch (type) {
    case FileUrlInfoType::kRedirectedFileUrl:
        return d->targetUrl;
    case FileUrlInfoType::kOriginalUrl:
        return d->originalUrl;
    default:
        return AbstractFileInfo::urlInfo(type);
    }
}
bool TrashFileInfo::canRename() const
{
    if (!d->dFileInfo)
        return false;

    bool value = false;
    bool success = false;
    value = d->dFileInfo->attribute(DFileInfo::AttributeID::kAccessCanRename, &success).toBool();
    return value;
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
    if (d->targetUrl.isValid()) {
        if (FileUtils::isDesktopFile(d->targetUrl)) {
            DesktopFileInfo dfi(d->targetUrl);
            return dfi.fileIcon();
        }
    }

    return AbstractFileInfo::fileIcon();
}

qint64 TrashFileInfo::size() const
{
    if (!d->dFileInfo)
        return qint64();

    qint64 size = 0;
    const QUrl &fileUrl = urlInfo(AbstractFileInfo::FileUrlInfoType::kUrl);
    if (FileUtils::isTrashRootFile(fileUrl)) {
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

QString TrashFileInfoPrivate::symLinkTarget() const
{
    if (!dFileInfo)
        return QString();

    QString symLinkTarget;
    bool success = false;
    symLinkTarget = dFileInfo->attribute(DFileInfo::AttributeID::kStandardSymlinkTarget, &success).toString();
    return symLinkTarget;
}

int TrashFileInfo::countChildFile() const
{
    if (FileUtils::isTrashRootFile(urlInfo(AbstractFileInfo::FileUrlInfoType::kUrl))) {
        if (d->dFileInfo)
            return d->dFileInfo->attribute(DFMIO::DFileInfo::AttributeID::kTrashItemCount).toInt();
    }

    if (isAttributes(AbstractFileInfo::FileIsType::kIsDir)) {
        DecoratorFileEnumerator enumerator(urlInfo(AbstractFileInfo::FileUrlInfoType::kUrl));
        return int(enumerator.fileCount());
    }

    return -1;
}

bool TrashFileInfo::isAttributes(const AbstractFileInfo::FileIsType type) const
{
    switch (type) {
    case FileIsType::kIsDir:
        if (FileUtils::isTrashRootFile(urlInfo(AbstractFileInfo::FileUrlInfoType::kUrl)))
            return true;
        return AbstractFileInfo::isAttributes(type);
    case FileIsType::kIsReadable:
        if (!d->dFileInfo)
            return false;

        if (d->targetUrl.isValid())
            return AbstractFileInfo::isAttributes(AbstractFileInfo::FileIsType::kIsReadable);

        return d->dFileInfo->attribute(DFileInfo::AttributeID::kAccessCanRead, nullptr).toBool();
    case FileIsType::kIsWritable:
        if (!d->dFileInfo)
            return false;

        if (d->targetUrl.isValid())
            return AbstractFileInfo::isAttributes(type);

        return d->dFileInfo->attribute(DFileInfo::AttributeID::kAccessCanWrite, nullptr).toBool();
    case FileIsType::kIsHidden:
        return false;
    case FileIsType::kIsSymLink:
        if (!d->dFileInfo)
            return false;

        return d->dFileInfo->attribute(DFileInfo::AttributeID::kStandardIsSymlink, nullptr).toBool();
    default:
        return AbstractFileInfo::isAttributes(type);
    }
}

bool TrashFileInfo::canDrop()
{
    return FileUtils::isTrashRootFile(urlInfo(AbstractFileInfo::FileUrlInfoType::kUrl));
}

bool TrashFileInfo::canHidden() const
{
    return false;
}

QVariant TrashFileInfo::timeInfo(const AbstractFileInfo::FileTimeType type) const
{
    switch (type) {
    case AbstractFileInfo::FileTimeType::kLastRead:
        return d->lastRead();
    case AbstractFileInfo::FileTimeType::kLastModified:
        return d->lastRead();
    case AbstractFileInfo::FileTimeType::kDeletionTime:
        return d->lastRead();
    default:
        return AbstractFileInfo::timeInfo(type);
    }
}

QVariant TrashFileInfo::customData(int role) const
{
    using namespace dfmbase::Global;
    if (role == kItemFileOriginalPath)
        return urlInfo(AbstractFileInfo::FileUrlInfoType::kOriginalUrl).path();
    else if (role == kItemFileDeletionDate)
        return d->deletionTime().toString(FileUtils::dateTimeFormat());
    else
        return QVariant();
}

}
