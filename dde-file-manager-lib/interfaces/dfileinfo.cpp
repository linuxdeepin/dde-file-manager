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

#include <QDateTime>
#include <QDir>
#include <QMimeDatabase>

DFileInfoPrivate::DFileInfoPrivate(const DUrl &url)
    : DAbstractFileInfoPrivate (url)
{
    fileInfo.setFile(url.toLocalFile());
}

DFileInfo::DFileInfo(const QString &filePath)
    : DFileInfo(DUrl::fromLocalFile(filePath))
{

}

DFileInfo::DFileInfo(const DUrl &fileUrl)
{
    d_ptr = getPrivateByUrl(fileUrl);
}

DFileInfo::DFileInfo(const QFileInfo &fileInfo)
    : DFileInfo(DUrl::fromLocalFile(fileInfo.absoluteFilePath()))
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

        UDiskDeviceInfo* info = deviceListener->getDeviceByFilePath(filePath());

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

QString DFileInfo::symLinkTarget()
{
    Q_D(const DFileInfo);

    return d->fileInfo.symLinkTarget();
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

    } else {
        return fileName();
    }
}

DFileInfo::DFileInfo()
{

}

DAbstractFileInfoPrivate *DFileInfo::createPrivateByUrl(const DUrl &url) const
{
    return new DFileInfoPrivate(url);
}
