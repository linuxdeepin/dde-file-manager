// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "private/trashfileinfo_p.h"
#include "utils/trashcorehelper.h"

#include <dfm-base/interfaces/private/fileinfo_p.h>
#include <dfm-base/dfm_global_defines.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/utils/fileutils.h>
#include <dfm-base/file/local/desktopfileinfo.h>
#include <dfm-base/utils/universalutils.h>
#include <dfm-base/base/standardpaths.h>

#include <dfm-io/denumerator.h>

#include <QCoreApplication>

DFMBASE_USE_NAMESPACE
namespace dfmplugin_trashcore {

TrashFileInfoPrivate::~TrashFileInfoPrivate()
{
}

QUrl TrashFileInfoPrivate::initTarget()
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

QString TrashFileInfoPrivate::fileName() const
{
    if (!dFileInfo)
        return QString();

    return dFileInfo->attribute(DFileInfo::AttributeID::kStandardName).toString();
}

QString TrashFileInfoPrivate::copyName() const
{
    if (!dFileInfo)
        return QString();

    if (targetUrl.isValid()) {
        if (FileUtils::isDesktopFileSuffix(targetUrl)) {
            DesktopFileInfo dfi(targetUrl);
            return dfi.nameOf(NameInfoType::kFileCopyName);
        }
    }

    return dFileInfo->attribute(DFileInfo::AttributeID::kStandardCopyName).toString();
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
    if (success) {
        time = QDateTime::fromSecsSinceEpoch(static_cast<uint>(data));
    } else {
        if (dAncestorsFileInfo)
            time = QDateTime::fromSecsSinceEpoch(static_cast<uint>(dAncestorsFileInfo->attribute(DFileInfo::AttributeID::kTimeAccess, &success).value<uint64_t>()));
    }
    return time;
}

QDateTime TrashFileInfoPrivate::lastModified() const
{
    if (!dFileInfo)
        return QDateTime();

    QDateTime time;
    bool success = false;
    uint64_t data = dFileInfo->attribute(DFileInfo::AttributeID::kTimeModified, &success).value<uint64_t>();
    if (success) {
        time = QDateTime::fromSecsSinceEpoch(static_cast<uint>(data));
    } else {
        if (dAncestorsFileInfo)
            time = QDateTime::fromSecsSinceEpoch(static_cast<uint>(dAncestorsFileInfo->attribute(DFileInfo::AttributeID::kTimeModified, &success).value<uint64_t>()));
    }
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
    : ProxyFileInfo(url), d(new TrashFileInfoPrivate(this))
{
    d->dFileInfo.reset(new DFileInfo(url));
    if (!d->dFileInfo) {
        fmWarning() << "dfm-io use factory create fileinfo Failed, url: " << url;
        return;
    }
    bool init = d->dFileInfo->initQuerier();
    if (!init) {
        //        fmWarning() << "querier init failed, url: " << url;
        return;
    }

    const QUrl &urlTarget = d->initTarget();
    if (urlTarget.isValid()) {
        d->targetUrl.setPath(urlTarget.path());
        setProxy(InfoFactory::create<FileInfo>(d->targetUrl));
    } else {
        if (!FileUtils::isTrashRootFile(url))
            fmWarning() << "create proxy failed, target url is invalid, url: " << url;
    }
}

TrashFileInfo::~TrashFileInfo()
{
}

bool TrashFileInfo::exists() const
{
    if (FileUtils::isTrashRootFile(urlOf(UrlInfoType::kUrl)))
        return true;

    if (d->dFileInfo)
        return d->dFileInfo->exists();

    return ProxyFileInfo::exists()
            || FileUtils::isTrashRootFile(urlOf(UrlInfoType::kUrl));
}

Qt::DropActions TrashFileInfo::supportedOfAttributes(const FileInfo::SupportType type) const
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

void TrashFileInfo::refresh()
{
    ProxyFileInfo::refresh();
}

QString TrashFileInfo::nameOf(const NameInfoType type) const
{
    switch (type) {
    case NameInfoType::kFileName:
        return d->fileName();
    case NameInfoType::kFileCopyName: {
        if (d->targetUrl.isValid()) {
            if (FileUtils::isDesktopFileSuffix(d->targetUrl)) {
                return d->copyName();
            }
        }
        return displayOf(DisPlayInfoType::kFileDisplayName);
    }
    case NameInfoType::kMimeTypeName:
        return d->mimeTypeName();
    default:
        return ProxyFileInfo::nameOf(type);
    }
}

QString TrashFileInfo::displayOf(const DisPlayInfoType type) const
{
    if (DisPlayInfoType::kFileDisplayName == type) {
        if (urlOf(UrlInfoType::kUrl) == TrashCoreHelper::rootUrl())
            return QCoreApplication::translate("PathManager", "Trash");

        if (!d->dFileInfo)
            return QString();

        if (d->targetUrl.isValid()) {
            if (FileUtils::isDesktopFileSuffix(d->targetUrl)) {
                DesktopFileInfo dfi(d->targetUrl);
                return dfi.displayOf(DisPlayInfoType::kFileDisplayName);
            }
        }

        return d->dFileInfo->attribute(DFileInfo::AttributeID::kStandardDisplayName).toString();
    }

    return ProxyFileInfo::displayOf(type);
}
QString TrashFileInfo::pathOf(const PathInfoType type) const
{
    switch (type) {
    case FilePathInfoType::kSymLinkTarget:
        return d->symLinkTarget();
    default:
        return ProxyFileInfo::pathOf(type);
    }
}

QUrl TrashFileInfo::urlOf(const UrlInfoType type) const
{
    switch (type) {
    case FileUrlInfoType::kRedirectedFileUrl:
        return d->targetUrl;
    case FileUrlInfoType::kCustomerStartUrl:
        [[fallthrough]];
    case FileUrlInfoType::kOriginalUrl:
        return d->originalUrl;
    case FileUrlInfoType::kUrl:
        return url;
    default:
        return ProxyFileInfo::urlOf(type);
    }
}

bool TrashFileInfo::canAttributes(const CanableInfoType type) const
{
    switch (type) {
    case FileCanType::kCanDelete:
        if (!d->dFileInfo)
            return false;

        return d->dFileInfo->attribute(DFileInfo::AttributeID::kAccessCanDelete, nullptr).toBool();
    case FileCanType::kCanTrash:
        if (!d->dFileInfo)
            return false;

        return d->dFileInfo->attribute(DFileInfo::AttributeID::kAccessCanTrash, nullptr).toBool();
    case FileCanType::kCanRename:
        if (!d->dFileInfo)
            return false;

        return d->dFileInfo->attribute(DFileInfo::AttributeID::kAccessCanRename, nullptr).toBool();
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

QFile::Permissions TrashFileInfo::permissions() const
{
    QFileDevice::Permissions ps;

    if (d->dFileInfo) {
        ps = static_cast<QFileDevice::Permissions>(static_cast<uint16_t>(d->dFileInfo->permissions()));
    }

    ps &= ~QFileDevice::WriteOwner;
    ps &= ~QFileDevice::WriteUser;
    ps &= ~QFileDevice::WriteGroup;
    ps &= ~QFileDevice::WriteOther;

    return ps;
}

QIcon TrashFileInfo::fileIcon()
{
    if (d->targetUrl.isValid()) {
        if (FileUtils::isDesktopFileSuffix(d->targetUrl)) {
            DesktopFileInfo dfi(d->targetUrl);
            return dfi.fileIcon();
        }
    }

    return ProxyFileInfo::fileIcon();
}

qint64 TrashFileInfo::size() const
{
    if (!d->dFileInfo)
        return qint64();

    qint64 size = 0;
    const QUrl &fileUrl = urlOf(UrlInfoType::kUrl);
    if (FileUtils::isTrashRootFile(fileUrl)) {
        auto data = TrashCoreHelper::calculateTrashRoot();
        return data.first;
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

bool TrashFileInfo::isAttributes(const OptInfoType type) const
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

QVariant TrashFileInfo::timeOf(const TimeInfoType type) const
{
    switch (type) {
    case TimeInfoType::kLastRead:
        return d->lastRead();
    case TimeInfoType::kLastModified:
        return d->lastModified();
    case TimeInfoType::kCustomerSupport:
        [[fallthrough]];
    case TimeInfoType::kDeletionTime:
        return d->deletionTime();
    default:
        return ProxyFileInfo::timeOf(type);
    }
}

QVariant TrashFileInfo::customData(int role) const
{
    using namespace dfmbase::Global;
    if (role == kItemFileOriginalPath)
        return urlOf(UrlInfoType::kOriginalUrl).path();
    else if (role == kItemFileDeletionDate)
        return d->deletionTime().toString(FileUtils::dateTimeFormat());
    else if (role == Global::ItemRoles::kItemFileRefreshIcon) {
        return ProxyFileInfo::customData(role);
    } else
        return QVariant();
}

}
