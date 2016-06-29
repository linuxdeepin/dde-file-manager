#include "fileinfo.h"

#include "../app/global.h"

#include "../shutil/iconprovider.h"

#include <QDateTime>
#include <QDir>
#include <QMimeDatabase>

QMap<DUrl, bool> FileInfo::canRenameCacheMap;

FileInfo::FileInfo()
    : AbstractFileInfo()
{

}

FileInfo::FileInfo(const QString &fileUrl)
    : AbstractFileInfo(fileUrl)
{

}

FileInfo::FileInfo(const DUrl &fileUrl)
    : AbstractFileInfo(fileUrl)
{

}

FileInfo::FileInfo(const QFileInfo &fileInfo)
    : AbstractFileInfo(QUrl::fromLocalFile(fileInfo.absoluteFilePath()))
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
    if (canRenameCacheMap.contains(fileUrl()))
        return canRenameCacheMap[fileUrl()];

    bool isWritable = this->isWritable();

    if (!isWritable && !exists(fileUrl())) {
        return false;
    }

    bool canRename = FileInfo(absolutePath()).isWritable() && isWritable;

    canRenameCacheMap[fileUrl()] = canRename;

    return canRename;
}

QString FileInfo::mimeTypeName() const
{
    if(data->mimeTypeName.isNull())
        data->mimeTypeName = mimeType(absoluteFilePath()).name();

    return data->mimeTypeName;
}

QIcon FileInfo::fileIcon() const
{
    return fileIconProvider->getFileIcon(absoluteFilePath(), mimeTypeName());
}

bool FileInfo::canIteratorDir() const
{
    return true;
}
