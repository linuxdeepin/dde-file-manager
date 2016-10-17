#include "fileinfo.h"

#include "app/define.h"

#include "shutil/iconprovider.h"

#include "controllers/pathmanager.h"

#include "widgets/singleton.h"

#include "app/define.h"
#include "widgets/singleton.h"
#include "usershare/usersharemanager.h"

#include <QDateTime>
#include <QDir>
#include <QMimeDatabase>

QMap<DUrl, bool> FileInfo::canRenameCacheMap;

FileInfo::FileInfo()
    : DAbstractFileInfo()
{

}

FileInfo::FileInfo(const QString &fileUrl)
    : DAbstractFileInfo(fileUrl)
{

}

FileInfo::FileInfo(const DUrl &fileUrl)
    : DAbstractFileInfo(fileUrl)
{

}

FileInfo::FileInfo(const QFileInfo &fileInfo)
    : DAbstractFileInfo(QUrl::fromLocalFile(fileInfo.absoluteFilePath()))
{

}

bool FileInfo::exists(const DUrl &fileUrl)
{
    return QFileInfo::exists(fileUrl.toLocalFile());
}

QMimeType FileInfo::mimeType(const QString &filePath)
{
    QMimeDatabase db;

    return db.mimeTypeForFile(filePath);
}

bool FileInfo::isCanRename() const
{
    if (systemPathManager->isSystemPath(absoluteFilePath()))
        return false;

    bool canRename = FileInfo(absolutePath()).isWritable();

    canRenameCacheMap[fileUrl()] = canRename;

    return canRename;
}

bool FileInfo::isCanShare() const
{
    return fileUrl().path().startsWith(QDir::homePath()) && isDir();
}

bool FileInfo::isShared() const
{
    return userShareManager->isShareFile(absoluteFilePath());
}

QMimeType FileInfo::mimeType() const
{
    if (!data->mimeType.isValid())
        data->mimeType = mimeType(absoluteFilePath());

    return data->mimeType;
}

QIcon FileInfo::fileIcon() const
{
    return fileIconProvider->getFileIcon(fileUrl(), mimeTypeName());
}

bool FileInfo::canIteratorDir() const
{
    return true;
}

QString FileInfo::subtitleForEmptyFloder() const
{
    if (!DAbstractFileInfo::exists()) {
        return QObject::tr("File has been moved or deleted");
    } else if (!isReadable()) {
        return QObject::tr("You do not have permission to access this folder");
    }

    return QObject::tr("Folder is empty");
}

QString FileInfo::displayName() const
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
