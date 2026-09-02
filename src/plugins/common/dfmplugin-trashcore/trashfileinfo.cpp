// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "private/trashfileinfo_p.h"
#include "utils/trashcorehelper.h"

#include <dfm-base/interfaces/private/fileinfo_p.h>
#include <dfm-base/dfm_global_defines.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/utils/fileutils.h>
#include <dfm-base/utils/trashutils.h>
#include <dfm-base/file/local/desktopfileinfo.h>
#include <dfm-base/utils/universalutils.h>
#include <dfm-base/base/standardpaths.h>

#include <QCoreApplication>
#include <QDir>

DFMBASE_USE_NAMESPACE
namespace dfmplugin_trashcore {

TrashFileInfoPrivate::~TrashFileInfoPrivate()
{
}

QUrl TrashFileInfoPrivate::initTarget()
{
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
        return cachedDeletionTime;

    return QDateTime::fromString(dFileInfo->attribute(DFileInfo::AttributeID::kTrashDeletionDate).toString(), Qt::ISODate);
}

TrashFileInfo::TrashFileInfo(const QUrl &url)
    : ProxyFileInfo(url), d(new TrashFileInfoPrivate(this))
{
    if (TrashUtils::isTrashRootFile(url)) {
        // Root trash URL: resolve to local trash files directory without dfm-io.
        d->targetUrl = QUrl::fromLocalFile(StandardPaths::location(StandardPaths::kTrashLocalFilesPath));
        setProxy(InfoFactory::create<FileInfo>(d->targetUrl));
        return;
    }

    // Non-root trash URL: resolve to local file path via .trashinfo metadata.
    // This bypasses gvfsd entirely, avoiding blocks on stale CIFS mounts.
    // No DFileInfo is created — all attribute queries are delegated to the proxy.
    TrashUtils::TrashItemInfo item = TrashUtils::resolveTrashUrl(url);
    if (!item.localFileUrl.isValid()) {
        fmWarning() << "Trash: Failed to resolve trash URL to local path:" << url;
        return;
    }

    d->targetUrl = item.localFileUrl;
    d->originalUrl = item.originalUrl;
    d->cachedDeletionTime = item.deletionTime;
    setProxy(InfoFactory::create<FileInfo>(d->targetUrl));
}

TrashFileInfo::~TrashFileInfo()
{
}

bool TrashFileInfo::exists() const
{
    if (TrashUtils::isTrashRootFile(urlOf(UrlInfoType::kUrl)))
        return true;

    if (d->dFileInfo)
        return d->dFileInfo->exists();

    return ProxyFileInfo::exists();
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
        if (d->dFileInfo)
            return d->fileName();
        return ProxyFileInfo::nameOf(type);
    case NameInfoType::kFileCopyName: {
        if (d->targetUrl.isValid()) {
            if (FileUtils::isDesktopFileSuffix(d->targetUrl)) {
                DesktopFileInfo dfi(d->targetUrl);
                return dfi.nameOf(NameInfoType::kFileCopyName);
            }
        }
        if (d->dFileInfo)
            return d->copyName();
        return ProxyFileInfo::nameOf(type);
    }
    case NameInfoType::kMimeTypeName:
        if (d->dFileInfo)
            return d->mimeTypeName();
        return ProxyFileInfo::nameOf(type);
    default:
        return ProxyFileInfo::nameOf(type);
    }
}

QString TrashFileInfo::displayOf(const DisPlayInfoType type) const
{
    if (DisPlayInfoType::kFileDisplayName == type) {
        if (urlOf(UrlInfoType::kUrl) == TrashCoreHelper::rootUrl())
            return QCoreApplication::translate("PathManager", "Trash");

        if (d->targetUrl.isValid()) {
            if (FileUtils::isDesktopFileSuffix(d->targetUrl)) {
                DesktopFileInfo dfi(d->targetUrl);
                return dfi.displayOf(DisPlayInfoType::kFileDisplayName);
            }
        }

        if (d->dFileInfo)
            return d->dFileInfo->attribute(DFileInfo::AttributeID::kStandardDisplayName).toString();
        return ProxyFileInfo::displayOf(type);
    }

    return ProxyFileInfo::displayOf(type);
}
QString TrashFileInfo::pathOf(const PathInfoType type) const
{
    switch (type) {
    case FilePathInfoType::kSymLinkTarget:
        if (d->dFileInfo)
            return d->symLinkTarget();
        return ProxyFileInfo::pathOf(type);
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
        if (d->dFileInfo)
            return d->dFileInfo->attribute(DFileInfo::AttributeID::kAccessCanDelete, nullptr).toBool();
        return ProxyFileInfo::canAttributes(type);
    case FileCanType::kCanTrash:
        if (d->dFileInfo)
            return d->dFileInfo->attribute(DFileInfo::AttributeID::kAccessCanTrash, nullptr).toBool();
        return ProxyFileInfo::canAttributes(type);
    case FileCanType::kCanRename:
        if (d->dFileInfo)
            return d->dFileInfo->attribute(DFileInfo::AttributeID::kAccessCanRename, nullptr).toBool();
        return ProxyFileInfo::canAttributes(type);
    case FileCanType::kCanDrop:
        return TrashUtils::isTrashRootFile(urlOf(UrlInfoType::kUrl));
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
    } else {
        ps = ProxyFileInfo::permissions();
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
    const QUrl &fileUrl = urlOf(UrlInfoType::kUrl);
    if (TrashUtils::isTrashRootFile(fileUrl)) {
        auto data = TrashCoreHelper::calculateTrashRoot();
        return data.first;
    }

    if (d->dFileInfo) {
        bool success = false;
        qint64 size = d->dFileInfo->attribute(DFileInfo::AttributeID::kStandardSize, &success).value<qint64>();
        return size;
    }

    return ProxyFileInfo::size();
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
    if (TrashUtils::isTrashRootFile(urlOf(UrlInfoType::kUrl))) {
        return TrashUtils::countTrashItems();
    }

    if (isAttributes(OptInfoType::kIsDir)) {
        QDir dir(d->targetUrl.toLocalFile());
        if (dir.exists())
            return static_cast<int>(dir.entryList(QDir::NoDotAndDotDot | QDir::AllEntries).size());
    }

    return -1;
}

bool TrashFileInfo::isAttributes(const OptInfoType type) const
{
    switch (type) {
    case FileIsType::kIsDir:
        if (TrashUtils::isTrashRootFile(urlOf(UrlInfoType::kUrl)))
            return true;
        return ProxyFileInfo::isAttributes(type);
    case FileIsType::kIsReadable:
        if (!d->dFileInfo)
            return ProxyFileInfo::isAttributes(type);
        if (d->targetUrl.isValid())
            return ProxyFileInfo::isAttributes(OptInfoType::kIsReadable);
        return d->dFileInfo->attribute(DFileInfo::AttributeID::kAccessCanRead, nullptr).toBool();
    case FileIsType::kIsWritable:
        if (!d->dFileInfo)
            return ProxyFileInfo::isAttributes(type);
        if (d->targetUrl.isValid())
            return ProxyFileInfo::isAttributes(type);
        return d->dFileInfo->attribute(DFileInfo::AttributeID::kAccessCanWrite, nullptr).toBool();
    case FileIsType::kIsHidden:
        return false;
    case FileIsType::kIsSymLink:
        if (!d->dFileInfo)
            return ProxyFileInfo::isAttributes(type);
        return d->dFileInfo->attribute(DFileInfo::AttributeID::kStandardIsSymlink, nullptr).toBool();
    default:
        return ProxyFileInfo::isAttributes(type);
    }
}

QVariant TrashFileInfo::timeOf(const TimeInfoType type) const
{
    switch (type) {
    case TimeInfoType::kLastRead:
        if (d->dFileInfo)
            return d->lastRead();
        return ProxyFileInfo::timeOf(type);
    case TimeInfoType::kLastModified:
        if (d->dFileInfo)
            return d->lastModified();
        return ProxyFileInfo::timeOf(type);
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
