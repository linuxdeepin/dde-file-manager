#include "fileinfo.h"

#include "../app/global.h"

#include "../shutil/iconprovider.h"

#include <QDateTime>
#include <QDir>
#include <QMimeDatabase>

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

void FileInfo::setFile(const DUrl &fileUrl)
{
    setUrl(fileUrl);
}

void FileInfo::setUrl(const DUrl &url)
{
    setFile(url);
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
    return QFileInfo(absolutePath()).isWritable()
            && isWritable();
}

QString FileInfo::mimeTypeName() const
{
    if(data->mimeTypeName.isEmpty())
        data->mimeTypeName = mimeType(absoluteFilePath()).name();

    return data->mimeTypeName;
}

QIcon FileInfo::fileIcon() const
{
    return fileIconProvider->getFileIcon(absoluteFilePath());
}
