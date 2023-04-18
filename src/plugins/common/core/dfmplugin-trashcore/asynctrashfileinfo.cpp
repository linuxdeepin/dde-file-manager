// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "asynctrashfileinfo.h"
#include "utils/trashcorehelper.h"

#include "dfm-base/interfaces/private/fileinfo_p.h"
#include "dfm-base/dfm_global_defines.h"
#include "dfm-base/base/schemefactory.h"
#include "dfm-base/utils/fileutils.h"
#include "dfm-base/file/local/desktopfileinfo.h"
#include "dfm-base/file/local/asyncfileinfo.h"
#include "dfm-base/utils/universalutils.h"
#include "dfm-base/base/standardpaths.h"
#include "dfm-base/utils/fileinfohelper.h"

#include <dfm-io/denumerator.h>

#include <QCoreApplication>

DFMBASE_USE_NAMESPACE
namespace dfmplugin_trashcore {
class AsyncTrashFileInfoPrivate
{
public:
    explicit AsyncTrashFileInfoPrivate(AsyncTrashFileInfo *qq)
        : q(qq)
    {
    }

    virtual ~AsyncTrashFileInfoPrivate();

    bool init();
    QUrl initTarget();
    QString fileName() const;
    QString copyName() const;
    QString mimeTypeName();
    QDateTime lastRead() const;
    QDateTime lastModified() const;
    QDateTime deletionTime() const;
    QString symLinkTarget() const;
    QVariant asyncAttribute(const AsyncFileInfo::AsyncAttributeID id);
    void cacheingAllAttributes();
    QString displayName();

    QSharedPointer<DFileInfo> dFileInfo { nullptr };
    QSharedPointer<DFileInfo> dAncestorsFileInfo { nullptr };
    QUrl targetUrl;
    QUrl originalUrl;
    QMap<AsyncFileInfo::AsyncAttributeID, QVariant> cacheAsyncAttributes;
    AsyncTrashFileInfo *const q;
    QReadWriteLock cacheLock;
    std::atomic_bool cacheing { false };
};

AsyncTrashFileInfoPrivate::~AsyncTrashFileInfoPrivate()
{
}

QUrl AsyncTrashFileInfoPrivate::initTarget()
{
    QVariant attributeTargetUri = dFileInfo->attribute(DFileInfo::AttributeID::kStandardTargetUri);
    if (!attributeTargetUri.toString().isEmpty())
        targetUrl = dFileInfo->attribute(DFileInfo::AttributeID::kStandardTargetUri).toUrl();

    originalUrl = QUrl::fromUserInput(dFileInfo->attribute(DFileInfo::AttributeID::kTrashOrigPath).toString());
    const bool urlIsRoot = UniversalUtils::urlEquals(TrashCoreHelper::rootUrl(), q->fileUrl());
    if (!targetUrl.isValid() && !urlIsRoot) {
        QUrl ancestors = q->fileUrl();
        while (TrashCoreHelper::rootUrl().isParentOf(ancestors)) {
            QUrl urlPre = ancestors;
            ancestors = UrlRoute::urlParent(ancestors);
            if (UniversalUtils::urlEquals(TrashCoreHelper::rootUrl(), ancestors)) {
                ancestors = urlPre;
                break;
            }
        }

        QSharedPointer<DFileInfo> fileinfo { new DFileInfo(ancestors) };
        if (fileinfo->initQuerier()) {
            const QUrl &ancestorsTargetUrl = fileinfo->attribute(DFileInfo::AttributeID::kStandardTargetUri).toUrl();
            if (ancestorsTargetUrl.isValid()) {
                QString localRootPath = ancestorsTargetUrl.toString();
                const QString &fileSuffix = q->fileUrl().path().mid(q->fileUrl().path().indexOf("/", 1));
                const QUrl &urlReal = localRootPath + fileSuffix;

                targetUrl = urlReal;
                QString localRootOriginalPath = fileinfo->attribute(DFileInfo::AttributeID::kTrashOrigPath).toString();
                originalUrl = QUrl::fromUserInput(localRootOriginalPath + fileSuffix);
                dAncestorsFileInfo = fileinfo;
                return urlReal;
            }
        }
    } else if (urlIsRoot) {
        const QUrl &urlTrashFiles = QUrl::fromLocalFile(StandardPaths::location(StandardPaths::kTrashLocalFilesPath));
        QSharedPointer<DFileInfo> fileinfo { new DFileInfo(urlTrashFiles) };
        if (fileinfo->initQuerier()) {
            targetUrl = urlTrashFiles;
            originalUrl = QUrl();
            dAncestorsFileInfo = fileinfo;
            return targetUrl;
        }
    }

    return targetUrl;
}

QString AsyncTrashFileInfoPrivate::fileName() const
{
    if (!dFileInfo)
        return QString();

    if (targetUrl.isValid()) {
        if (FileUtils::isDesktopFile(targetUrl)) {
            DesktopFileInfo dfi(targetUrl);
            return dfi.displayOf(DisPlayInfoType::kFileDisplayName);
        }
    }

    return dFileInfo->attribute(DFileInfo::AttributeID::kStandardName).toString();
}

QString AsyncTrashFileInfoPrivate::copyName() const
{
    if (!dFileInfo)
        return QString();

    if (targetUrl.isValid()) {
        if (FileUtils::isDesktopFile(targetUrl)) {
            DesktopFileInfo dfi(targetUrl);
            return dfi.nameOf(NameInfoType::kFileCopyName);
        }
    }

    return dFileInfo->attribute(DFileInfo::AttributeID::kStandardCopyName).toString();
}

QString AsyncTrashFileInfoPrivate::mimeTypeName()
{
    if (!dFileInfo)
        return QString();

    QString type;
    bool success = false;
    type = dFileInfo->attribute(DFileInfo::AttributeID::kStandardContentType, &success).toString();
    return type;
}

QDateTime AsyncTrashFileInfoPrivate::lastRead() const
{
    if (!dFileInfo)
        return QDateTime();

    QDateTime time;
    bool success = false;
    uint64_t data = dFileInfo->attribute(DFileInfo::AttributeID::kTimeAccess, &success).value<uint64_t>();
    if (success) {
        time = QDateTime::fromTime_t(static_cast<uint>(data));
    } else {
        if (dAncestorsFileInfo)
            time = QDateTime::fromTime_t(static_cast<uint>(dAncestorsFileInfo->attribute(DFileInfo::AttributeID::kTimeAccess, &success).value<uint64_t>()));
    }
    return time;
}

QDateTime AsyncTrashFileInfoPrivate::lastModified() const
{
    if (!dFileInfo)
        return QDateTime();

    QDateTime time;
    bool success = false;
    uint64_t data = dFileInfo->attribute(DFileInfo::AttributeID::kTimeModified, &success).value<uint64_t>();
    if (success) {
        time = QDateTime::fromTime_t(static_cast<uint>(data));
    } else {
        if (dAncestorsFileInfo)
            time = QDateTime::fromTime_t(static_cast<uint>(dAncestorsFileInfo->attribute(DFileInfo::AttributeID::kTimeModified, &success).value<uint64_t>()));
    }
    return time;
}

QDateTime AsyncTrashFileInfoPrivate::deletionTime() const
{
    if (dAncestorsFileInfo)
        return QDateTime::fromString(dAncestorsFileInfo->attribute(DFileInfo::AttributeID::kTrashDeletionDate).toString(), Qt::ISODate);

    if (!dFileInfo)
        return QDateTime();

    return QDateTime::fromString(dFileInfo->attribute(DFileInfo::AttributeID::kTrashDeletionDate).toString(), Qt::ISODate);
}

AsyncTrashFileInfo::AsyncTrashFileInfo(const QUrl &url)
    : ProxyFileInfo(url), d(new AsyncTrashFileInfoPrivate(this))
{
}

AsyncTrashFileInfo::~AsyncTrashFileInfo()
{
}

bool AsyncTrashFileInfo::initQuerier()
{
    if (d->cacheing)
        return false;
    d->cacheing = true;
    d->dFileInfo.reset(new DFileInfo(url));
    if (!d->dFileInfo) {
        qWarning() << "dfm-io use factory create fileinfo Failed, url: " << url;
        d->cacheing = false;
        return false;
    }
    bool init = d->dFileInfo->initQuerier();
    if (!init) {
        //        qWarning() << "querier init failed, url: " << url;
        d->cacheing = false;
        return false;
    }

    const QUrl &urlTarget = d->initTarget();
    if (urlTarget.isValid()) {
        setProxy(InfoFactory::create<FileInfo>(urlTarget));
    } else {
        if (!FileUtils::isTrashRootFile(url))
            qWarning() << "create proxy failed, target url is invalid, url: " << url;
        return false;
    }

    d->cacheingAllAttributes();

    d->cacheing = false;
    return true;
}

bool AsyncTrashFileInfo::exists() const
{
    if (FileUtils::isTrashRootFile(urlOf(UrlInfoType::kUrl)))
        return true;

    return d->asyncAttribute(AsyncFileInfo::AsyncAttributeID::kStandardFileExists).toBool();
}

Qt::DropActions AsyncTrashFileInfo::supportedOfAttributes(const FileInfo::SupportType type) const
{
    switch (type) {
    case FileInfo::SupportType::kDrop: {
        const QString &path = url.path();

        return path.isEmpty() || path == "/" ? Qt::MoveAction : Qt::IgnoreAction;
    }
    case FileInfo::SupportType::kDrag:
        return Qt::CopyAction | Qt::MoveAction;
    default:
        return ProxyFileInfo::supportedOfAttributes(type);
    }
}

void AsyncTrashFileInfo::refresh()
{
    FileInfoHelper::instance().fileRefreshAsync(this->sharedFromThis());
    ProxyFileInfo::refresh();
}

QString AsyncTrashFileInfo::nameOf(const NameInfoType type) const
{
    switch (type) {
    case NameInfoType::kFileName:
        return d->asyncAttribute(AsyncFileInfo::AsyncAttributeID::kStandardFileName).toString();
    case NameInfoType::kFileCopyName: {
        if (d->targetUrl.isValid()) {
            if (FileUtils::isDesktopFile(d->targetUrl)) {
                return d->asyncAttribute(AsyncFileInfo::AsyncAttributeID::kStandardCopyName).toString();
            }
        }
        return displayOf(DisPlayInfoType::kFileDisplayName);
    }
    case NameInfoType::kMimeTypeName:
        return d->asyncAttribute(AsyncFileInfo::AsyncAttributeID::kStandardContentType).toString();
    default:
        return ProxyFileInfo::nameOf(type);
    }
}

QString AsyncTrashFileInfo::displayOf(const DisPlayInfoType type) const
{
    if (DisPlayInfoType::kFileDisplayName == type) {
        return d->asyncAttribute(AsyncFileInfo::AsyncAttributeID::kStandardDisplayName).toString();
    }

    return ProxyFileInfo::displayOf(type);
}
QString AsyncTrashFileInfo::pathOf(const PathInfoType type) const
{
    switch (type) {
    case FilePathInfoType::kSymLinkTarget:
        return d->asyncAttribute(AsyncFileInfo::AsyncAttributeID::kStandardSymlinkTarget).toString();
    default:
        return ProxyFileInfo::pathOf(type);
    }
}

QUrl AsyncTrashFileInfo::urlOf(const UrlInfoType type) const
{
    switch (type) {
    case FileUrlInfoType::kRedirectedFileUrl:
        return d->targetUrl;
    case FileUrlInfoType::kOriginalUrl:
        return d->originalUrl;
    case FileUrlInfoType::kUrl:
        return url;
    default:
        return ProxyFileInfo::urlOf(type);
    }
}

bool AsyncTrashFileInfo::canAttributes(const CanableInfoType type) const
{
    switch (type) {
    case FileCanType::kCanDelete:
        if (!d->dFileInfo)
            return false;

        return d->asyncAttribute(AsyncFileInfo::AsyncAttributeID::kAccessCanDelete).toBool();
    case FileCanType::kCanTrash:
        if (!d->dFileInfo)
            return false;

        return d->asyncAttribute(AsyncFileInfo::AsyncAttributeID::kAccessCanTrash).toBool();
    case FileCanType::kCanRename:
        if (!d->dFileInfo)
            return false;

        return d->asyncAttribute(AsyncFileInfo::AsyncAttributeID::kAccessCanRename).toBool();
    case FileCanType::kCanDrop:
        return FileUtils::isTrashRootFile(urlOf(UrlInfoType::kUrl));
    case FileCanType::kCanHidden:
        return false;
    case FileCanType::kCanRedirectionFileUrl:
        return true;
    default:
        return ProxyFileInfo::canAttributes(type);
    }
}

QFile::Permissions AsyncTrashFileInfo::permissions() const
{
    QFileDevice::Permissions p = d->asyncAttribute(AsyncFileInfo::AsyncAttributeID::kAccessPermissions).value<QFileDevice::Permissions>();

    p &= ~QFileDevice::WriteOwner;
    p &= ~QFileDevice::WriteUser;
    p &= ~QFileDevice::WriteGroup;
    p &= ~QFileDevice::WriteOther;

    return p;
}

QIcon AsyncTrashFileInfo::fileIcon()
{
    if (d->targetUrl.isValid() && FileUtils::isDesktopFile(d->targetUrl)) {
        auto dfi = InfoFactory::create<FileInfo>(d->targetUrl);
        if (dfi)
            return dfi->fileIcon();
    }

    return ProxyFileInfo::fileIcon();
}

qint64 AsyncTrashFileInfo::size() const
{
    if (!d->dFileInfo)
        return qint64();

    qint64 size = 0;
    const QUrl &fileUrl = urlOf(UrlInfoType::kUrl);
    if (FileUtils::isTrashRootFile(fileUrl)) {
        return d->asyncAttribute(AsyncFileInfo::AsyncAttributeID::kTrashItemCount).value<qint64>();
    }

    size = d->asyncAttribute(AsyncFileInfo::AsyncAttributeID::kStandardSize).value<qint64>();
    return size;
}

QString AsyncTrashFileInfoPrivate::symLinkTarget() const
{
    if (!dFileInfo)
        return QString();

    QString symLinkTarget;
    bool success = false;
    symLinkTarget = dFileInfo->attribute(DFileInfo::AttributeID::kStandardSymlinkTarget, &success).toString();
    return symLinkTarget;
}

QVariant AsyncTrashFileInfoPrivate::asyncAttribute(const AsyncFileInfo::AsyncAttributeID id)
{
    QReadLocker lk(&cacheLock);
    return cacheAsyncAttributes.value(id);
}

void AsyncTrashFileInfoPrivate::cacheingAllAttributes()
{
    if (!dFileInfo)
        return;
    QMap<AsyncFileInfo::AsyncAttributeID, QVariant> tmp;
    tmp.insert(AsyncFileInfo::AsyncAttributeID::kStandardFileExists, dFileInfo->exists());
    tmp.insert(AsyncFileInfo::AsyncAttributeID::kStandardFileName, fileName());
    tmp.insert(AsyncFileInfo::AsyncAttributeID::kStandardCopyName, copyName());
    tmp.insert(AsyncFileInfo::AsyncAttributeID::kStandardContentType, mimeTypeName());
    tmp.insert(AsyncFileInfo::AsyncAttributeID::kStandardDisplayName, displayName());
    tmp.insert(AsyncFileInfo::AsyncAttributeID::kStandardSymlinkTarget, symLinkTarget());

    tmp.insert(AsyncFileInfo::AsyncAttributeID::kAccessCanDelete,
               dFileInfo->attribute(DFileInfo::AttributeID::kAccessCanDelete, nullptr));
    tmp.insert(AsyncFileInfo::AsyncAttributeID::kAccessCanTrash,
               dFileInfo->attribute(DFileInfo::AttributeID::kAccessCanTrash, nullptr));
    tmp.insert(AsyncFileInfo::AsyncAttributeID::kAccessCanRename,
               dFileInfo->attribute(DFileInfo::AttributeID::kAccessCanRename, nullptr));
    tmp.insert(AsyncFileInfo::AsyncAttributeID::kAccessPermissions, QVariant::fromValue(dFileInfo->permissions()));
    tmp.insert(AsyncFileInfo::AsyncAttributeID::kStandardSize,
               dFileInfo->attribute(DFileInfo::AttributeID::kStandardSize));
    const QUrl &fileUrl = q->urlOf(UrlInfoType::kUrl);
    if (FileUtils::isTrashRootFile(fileUrl)) {
        auto data = TrashCoreHelper::calculateTrashRoot();
        tmp.insert(AsyncFileInfo::AsyncAttributeID::kTrashItemCount, data.first);
    }

    QWriteLocker lk(&cacheLock);
    cacheAsyncAttributes = tmp;
}

QString AsyncTrashFileInfoPrivate::displayName()
{
    if (q->urlOf(UrlInfoType::kUrl) == TrashCoreHelper::rootUrl())
        return QCoreApplication::translate("PathManager", "Trash");

    if (!dFileInfo)
        return QString();

    if (targetUrl.isValid()) {
        if (FileUtils::isDesktopFile(targetUrl)) {
            DesktopFileInfo dfi(targetUrl);
            return dfi.displayOf(DisPlayInfoType::kFileDisplayName);
        }
    }

    return dFileInfo->attribute(DFileInfo::AttributeID::kStandardDisplayName).toString();
}

int AsyncTrashFileInfo::countChildFile() const
{
    if (FileUtils::isTrashRootFile(urlOf(UrlInfoType::kUrl))) {
        DFileInfo trashRootFileInfo(FileUtils::trashRootUrl());
        return trashRootFileInfo.attribute(DFMIO::DFileInfo::AttributeID::kTrashItemCount).toInt();
    }

    if (isAttributes(OptInfoType::kIsDir)) {
        DFMIO::DEnumerator enumerator(urlOf(UrlInfoType::kUrl));
        return int(enumerator.fileCount());
    }

    return -1;
}

bool AsyncTrashFileInfo::isAttributes(const OptInfoType type) const
{
    switch (type) {
    case FileIsType::kIsDir:
        if (FileUtils::isTrashRootFile(urlOf(UrlInfoType::kUrl)))
            return true;
        return ProxyFileInfo::isAttributes(type);
    case FileIsType::kIsReadable:
        if (!d->dFileInfo)
            return false;

        if (d->targetUrl.isValid())
            return ProxyFileInfo::isAttributes(OptInfoType::kIsReadable);

        return d->dFileInfo->attribute(DFileInfo::AttributeID::kAccessCanRead, nullptr).toBool();
    case FileIsType::kIsWritable:
        if (!d->dFileInfo)
            return false;

        if (d->targetUrl.isValid())
            return ProxyFileInfo::isAttributes(type);

        return d->dFileInfo->attribute(DFileInfo::AttributeID::kAccessCanWrite, nullptr).toBool();
    case FileIsType::kIsHidden:
        return false;
    case FileIsType::kIsSymLink:
        if (!d->dFileInfo)
            return false;

        return d->dFileInfo->attribute(DFileInfo::AttributeID::kStandardIsSymlink, nullptr).toBool();
    default:
        return ProxyFileInfo::isAttributes(type);
    }
}

QVariant AsyncTrashFileInfo::timeOf(const TimeInfoType type) const
{
    switch (type) {
    case TimeInfoType::kLastRead:
        return d->lastRead();
    case TimeInfoType::kLastModified:
        return d->lastModified();
    case TimeInfoType::kDeletionTime:
        return d->deletionTime();
    default:
        return ProxyFileInfo::timeOf(type);
    }
}

QVariant AsyncTrashFileInfo::customData(int role) const
{
    using namespace dfmbase::Global;
    if (role == kItemFileOriginalPath)
        return urlOf(UrlInfoType::kOriginalUrl).path();
    else if (role == kItemFileDeletionDate)
        return d->deletionTime().toString(FileUtils::dateTimeFormat());
    else
        return QVariant();
}

}

QUrl dfmplugin_trashcore::AsyncTrashFileInfo::fileUrl() const
{
    return url;
}
