#include "dfileinfo.h"
#include "private/dfileinfo_p.h"
#include "app/define.h"

#include "shutil/fileutils.h"

#include "controllers/pathmanager.h"

#include "singleton.h"

#include "app/define.h"
#include "singleton.h"
#include "usershare/usersharemanager.h"
#include "deviceinfo/udisklistener.h"

#include "dfileservices.h"
#include "dthumbnailprovider.h"
#include "dfileiconprovider.h"
#include "dmimedatabase.h"

#include <QDateTime>
#include <QDir>
#include <QPainter>
#include <QApplication>

#include <sys/stat.h>

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

QMimeType DFileInfo::mimeType(const QString &filePath, QMimeDatabase::MatchMode mode)
{
    DMimeDatabase db;

    return db.mimeTypeForFile(filePath, mode);
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

QString DFileInfo::fileSharedName() const
{
    const ShareInfo &info = userShareManager->getShareInfoByPath(absoluteFilePath());

    return info.shareName();
}

bool DFileInfo::canRename() const
{
    if (systemPathManager->isSystemPath(absoluteFilePath()))
        return false;

    bool canRename = DFileInfo(absolutePath()).isWritable();

//    canRenameCacheMap[fileUrl()] = canRename;

    return canRename;
}

bool DFileInfo::canShare() const
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

bool DFileInfo::canFetch() const
{
    return isDir() || FileUtils::isArchive(absoluteFilePath());
}

bool DFileInfo::isReadable() const
{
    Q_D(const DFileInfo);

    if (FileUtils::isGvfsMountFile(absoluteFilePath())){
        return true;
    }else{
        return d->fileInfo.isReadable();
    }
}

bool DFileInfo::isWritable() const
{
    Q_D(const DFileInfo);
    if (FileUtils::isGvfsMountFile(absoluteFilePath())){
        return true;
    }else{
        return d->fileInfo.isWritable();
    }
}

bool DFileInfo::isExecutable() const
{
    Q_D(const DFileInfo);
    if (FileUtils::isGvfsMountFile(absoluteFilePath())){
        return true;
    }else{
        return d->fileInfo.isExecutable();
    }
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

bool DFileInfo::isWritableShared() const
{
    const ShareInfo &info = userShareManager->getShareInfoByPath(absoluteFilePath());

    return info.isWritable();
}

bool DFileInfo::isAllowGuestShared() const
{
    const ShareInfo &info = userShareManager->getShareInfoByPath(absoluteFilePath());

    return info.isGuestOk();
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

    if (isSymLink() && !exists()) {
        struct stat attrib;

        if (lstat(d->fileInfo.filePath().toLocal8Bit().constData(), &attrib) >= 0)
            return QDateTime::fromTime_t(attrib.st_mtime);
    }

    return d->fileInfo.lastModified();
}

QDateTime DFileInfo::lastRead() const
{
    Q_D(const DFileInfo);

    return d->fileInfo.lastRead();
}

QMimeType DFileInfo::mimeType(QMimeDatabase::MatchMode mode) const
{
    Q_D(const DFileInfo);

    if (!d->mimeType.isValid() || d->mimeTypeMode != mode) {
        d->mimeType = mimeType(absoluteFilePath(), mode);
        d->mimeTypeMode = mode;
    }

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
    bool has_thumbnail = FileUtils::isGvfsMountFile(absoluteFilePath()) || DThumbnailProvider::instance()->hasThumbnail(d->fileInfo);

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
                        me->d_func()->icon = DFileIconProvider::globalProvider()->icon(*me.constData());
                });
                me->d_func()->requestingThumbnail = true;
                timer->deleteLater();
            });

            QMetaObject::invokeMethod(timer, "start", Qt::QueuedConnection);
        }

        return DFileIconProvider::globalProvider()->icon(*this);
    }

    if (isSymLink()) {
        const DUrl &symLinkTarget = this->symLinkTarget();

        if (symLinkTarget != fileUrl) {
            const DAbstractFileInfoPointer &fileInfo = DFileService::instance()->createFileInfo(Q_NULLPTR, symLinkTarget);

            if (fileInfo){
                d->icon = fileInfo->fileIcon();
                return d->icon;
            }
        }
    }

    d->icon = DFileIconProvider::globalProvider()->icon(*this);

    return d->icon;
}

QString DFileInfo::iconName() const
{
    if (systemPathManager->isSystemPath(absoluteFilePath()))
        return systemPathManager->getSystemPathIconNameByPath(absoluteFilePath());

    return DAbstractFileInfo::iconName();
}

QFileInfo DFileInfo::toQFileInfo() const
{
    Q_D(const DFileInfo);

    return d->fileInfo;
}

QDiskInfo DFileInfo::getDiskinfo()
{
    QDiskInfo info;
    qDebug() << this->filePath();
    UDiskDeviceInfoPointer uDiskDeviceInfoPointer = deviceListener->getDeviceByPath(this->filePath());
    if(!uDiskDeviceInfoPointer){
        uDiskDeviceInfoPointer = deviceListener->getDeviceByFilePath(this->filePath());
    }
    if (uDiskDeviceInfoPointer){
        info = uDiskDeviceInfoPointer->getDiskInfo();
    }
    return info;
}

DFileInfo::DFileInfo(DFileInfoPrivate &dd)
    : DAbstractFileInfo(dd)
{

}
