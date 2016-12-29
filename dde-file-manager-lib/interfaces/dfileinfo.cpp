#include "dfileinfo.h"
#include "private/dfileinfo_p.h"
#include "app/define.h"

#include "shutil/iconprovider.h"
#include "shutil/fileutils.h"

#include "controllers/pathmanager.h"

#include "widgets/singleton.h"

#include "app/define.h"
#include "widgets/singleton.h"
#include "usershare/usersharemanager.h"
#include "deviceinfo/udisklistener.h"

#include "dfileservices.h"
#include "dthumbnailprovider.h"

#include <QDateTime>
#include <QDir>
#include <QMimeDatabase>
#include <QPainter>

DFM_USE_NAMESPACE

#define REQUEST_THUMBNAIL_DEALY 500

DFileInfoPrivate::DFileInfoPrivate(const DUrl &url, DFileInfo *qq, bool hasCache)
    : DAbstractFileInfoPrivate (url, qq, hasCache)
{
    fileInfo.setFile(url.toLocalFile());
}

DFileInfo::DFileInfo(const QString &filePath, bool hasCache)
    : DFileInfo(DUrl::fromLocalFile(filePath), hasCache)
{

}

DFileInfo::DFileInfo(const DUrl &fileUrl, bool hasCache)
    : DAbstractFileInfo(*new DFileInfoPrivate(fileUrl, this, hasCache))
{

}

DFileInfo::DFileInfo(const QFileInfo &fileInfo, bool hasCache)
    : DFileInfo(DUrl::fromLocalFile(fileInfo.absoluteFilePath()), hasCache)
{

}

bool DFileInfo::exists(const DUrl &fileUrl)
{
    return QFileInfo::exists(fileUrl.toLocalFile());
}

QMimeType DFileInfo::mimeType(const QString &filePath)
{
    QMimeDatabase db;

    return db.mimeTypeForFile(filePath);
}

bool DFileInfo::exists() const
{
    Q_D(const DFileInfo);

    return d->fileInfo.exists();
}

QString DFileInfo::path() const
{
    Q_D(const DFileInfo);

    return d->fileInfo.path();
}

QString DFileInfo::filePath() const
{
    Q_D(const DFileInfo);

    return d->fileInfo.filePath();
}

QString DFileInfo::absolutePath() const
{
    Q_D(const DFileInfo);

    return d->fileInfo.absolutePath();
}

QString DFileInfo::absoluteFilePath() const
{
    Q_D(const DFileInfo);

    return d->fileInfo.absoluteFilePath();
}

QString DFileInfo::fileName() const
{
    Q_D(const DFileInfo);

    if (d->fileInfo.absoluteFilePath().endsWith(QDir::separator()))
        return QFileInfo(d->fileInfo.absolutePath()).fileName();

    return d->fileInfo.fileName();
}

bool DFileInfo::isCanRename() const
{
    if (systemPathManager->isSystemPath(absoluteFilePath()))
        return false;

    bool canRename = DFileInfo(absolutePath()).isWritable();

//    canRenameCacheMap[fileUrl()] = canRename;

    return canRename;
}

bool DFileInfo::isCanShare() const
{
    if (isDir() && isReadable()) {
        if (absoluteFilePath().startsWith(QDir::homePath())) {
            return true;
        }

        UDiskDeviceInfoPointer info = deviceListener->getDeviceByFilePath(filePath());

        if (info) {
            if (info->getMediaType() != UDiskDeviceInfo::unknown && info->getMediaType() !=UDiskDeviceInfo::network)
                return true;
        }
    }

    return false;
}

bool DFileInfo::isReadable() const
{
    Q_D(const DFileInfo);

    return d->fileInfo.isReadable();
}

bool DFileInfo::isWritable() const
{
    Q_D(const DFileInfo);

    return d->fileInfo.isWritable();
}

bool DFileInfo::isExecutable() const
{
    Q_D(const DFileInfo);

    return d->fileInfo.isExecutable();
}

bool DFileInfo::isHidden() const
{
    Q_D(const DFileInfo);

    return d->fileInfo.isHidden();
}

bool DFileInfo::isRelative() const
{
    Q_D(const DFileInfo);

    return d->fileInfo.isRelative();
}

bool DFileInfo::isAbsolute() const
{
    Q_D(const DFileInfo);

    return d->fileInfo.isAbsolute();
}

bool DFileInfo::isShared() const
{
    return userShareManager->isShareFile(absoluteFilePath());
}

bool DFileInfo::makeAbsolute()
{
    Q_D(DFileInfo);

    return d->fileInfo.makeAbsolute();
}

bool DFileInfo::isFile() const
{
    Q_D(const DFileInfo);

    return d->fileInfo.isFile();
}

bool DFileInfo::isDir() const
{
    Q_D(const DFileInfo);

    return d->fileInfo.isDir();
}

bool DFileInfo::isSymLink() const
{
    Q_D(const DFileInfo);

    return d->fileInfo.isSymLink();
}

DUrl DFileInfo::symLinkTarget() const
{
    Q_D(const DFileInfo);

    if (d->fileInfo.isSymLink())
        return DUrl::fromLocalFile(d->fileInfo.symLinkTarget());

    return DAbstractFileInfo::symLinkTarget();
}

QString DFileInfo::owner() const
{
    Q_D(const DFileInfo);

    return d->fileInfo.owner();
}

uint DFileInfo::ownerId() const
{
    Q_D(const DFileInfo);

    return d->fileInfo.ownerId();
}

QString DFileInfo::group() const
{
    Q_D(const DFileInfo);

    return d->fileInfo.group();
}

uint DFileInfo::groupId() const
{
    Q_D(const DFileInfo);

    return d->fileInfo.groupId();
}

bool DFileInfo::permission(QFileDevice::Permissions permissions) const
{
    Q_D(const DFileInfo);

    return d->fileInfo.permission(permissions);
}

QFileDevice::Permissions DFileInfo::permissions() const
{
    Q_D(const DFileInfo);

    return d->fileInfo.permissions();
}

qint64 DFileInfo::size() const
{
    Q_D(const DFileInfo);

    return d->fileInfo.size();
}

int DFileInfo::filesCount() const
{
    if (isDir())
        return FileUtils::filesCount(absoluteFilePath());

    return -1;
}

QDateTime DFileInfo::created() const
{
    Q_D(const DFileInfo);

    return d->fileInfo.created();
}

QDateTime DFileInfo::lastModified() const
{
    Q_D(const DFileInfo);

    return d->fileInfo.lastModified();
}

QDateTime DFileInfo::lastRead() const
{
    Q_D(const DFileInfo);

    return d->fileInfo.lastRead();
}

QMimeType DFileInfo::mimeType() const
{
    Q_D(const DFileInfo);

    if (!d->mimeType.isValid())
        d->mimeType = mimeType(absoluteFilePath());

    return d->mimeType;
}

bool DFileInfo::canIteratorDir() const
{
    return true;
}

QString DFileInfo::subtitleForEmptyFloder() const
{
    if (!exists()) {
        return QObject::tr("File has been moved or deleted");
    } else if (!isReadable()) {
        return QObject::tr("You do not have permission to access this folder");
    }

    return QObject::tr("Folder is empty");
}

QString DFileInfo::fileDisplayName() const
{
    if (systemPathManager->isSystemPath(filePath())) {
        QString displayName = systemPathManager->getSystemPathDisplayNameByPath(filePath());

        if (displayName.isEmpty())
            return fileName();
        else
            return displayName;

    }else if (deviceListener->isDeviceFolder(filePath())){
        UDiskDeviceInfoPointer deviceInfo = deviceListener->getDeviceByPath(filePath());
        if (!deviceInfo->fileDisplayName().isEmpty())
            return deviceInfo->fileDisplayName();
        else
            return fileName();
    }
    else {
        return fileName();
    }
}

void DFileInfo::refresh()
{
    Q_D(DFileInfo);

    d->fileInfo.refresh();
    d->icon = QIcon();
}

DUrl DFileInfo::goToUrlWhenDeleted() const
{
    if (deviceListener->isInDeviceFolder(absoluteFilePath()))
        return DUrl::fromLocalFile(QDir::homePath());

    return DAbstractFileInfo::goToUrlWhenDeleted();
}

void DFileInfo::makeToInactive()
{
    Q_D(DFileInfo);

    if (d->getIconTimer) {
        d->getIconTimer->stop();
    } else if (d->requestingThumbnail) {
        d->requestingThumbnail = false;
        DThumbnailProvider::instance()->removeInProduceQueue(d->fileInfo, DThumbnailProvider::Large);
    }
}

QIcon DFileInfo::fileIcon() const
{
    Q_D(const DFileInfo);

    if (!d->icon.isNull())
        return d->icon;

    const DUrl &fileUrl = this->fileUrl();
    bool has_thumbnail = DThumbnailProvider::instance()->hasThumbnail(d->fileInfo);

    if (has_thumbnail) {
        const QIcon icon(DThumbnailProvider::instance()->thumbnailFilePath(d->fileInfo, DThumbnailProvider::Large));

        if (!icon.isNull()) {
            QPixmap pixmap = icon.pixmap(DThumbnailProvider::Large, DThumbnailProvider::Large);
            QPainter pa(&pixmap);

            pa.setPen(Qt::gray);
            pa.drawRect(pixmap.rect().adjusted(0, 0, -1, -1));
            d->icon.addPixmap(pixmap);

            return d->icon;
        }

        if (d->getIconTimer) {
            QMetaObject::invokeMethod(d->getIconTimer, "start", Qt::QueuedConnection);
        } else {
            QTimer *timer = new QTimer();
            const QExplicitlySharedDataPointer<DFileInfo> me(const_cast<DFileInfo*>(this));

            d->getIconTimer = timer;
            timer->setSingleShot(true);
            timer->moveToThread(qApp->thread());
            timer->setInterval(REQUEST_THUMBNAIL_DEALY);

            QObject::connect(timer, &QTimer::timeout, timer, [fileUrl, timer, me] {
                DThumbnailProvider::instance()->appendToProduceQueue(me->d_func()->fileInfo, DThumbnailProvider::Large,
                                                                     [me] (const QString &path) {
                    if (path.isEmpty())
                        me->d_func()->icon = me->DAbstractFileInfo::fileIcon();
                });
                me->d_func()->requestingThumbnail = true;
                timer->deleteLater();
            });

            QMetaObject::invokeMethod(timer, "start", Qt::QueuedConnection);
        }

        return DAbstractFileInfo::fileIcon();
    }

    if (isSymLink()) {
        const DUrl &symLinkTarget = this->symLinkTarget();

        if (symLinkTarget != fileUrl) {
            const DAbstractFileInfoPointer &fileInfo = DFileService::instance()->createFileInfo(symLinkTarget);

            d->icon = fileInfo->fileIcon();

            return d->icon;
        }
    }

    d->icon = DAbstractFileInfo::fileIcon();

    return d->icon;
}

DFileInfo::DFileInfo(DFileInfoPrivate &dd)
    : DAbstractFileInfo(dd)
{

}
