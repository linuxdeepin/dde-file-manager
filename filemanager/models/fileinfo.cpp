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

FileInfo::FileInfo(const QUrl &fileUrl)
    : AbstractFileInfo(fileUrl)
{

}

FileInfo::FileInfo(const QFileInfo &fileInfo)
    : AbstractFileInfo(QUrl::fromLocalFile(fileInfo.absoluteFilePath()))
{

}

void FileInfo::setFile(const QString &fileUrl)
{
    setUrl(fileUrl);
}

bool FileInfo::exists(const QString &fileUrl)
{
    QUrl url(fileUrl);

    if(url.scheme().isEmpty())
        url = QUrl::fromLocalFile(fileUrl);

    return QFileInfo::exists(url.toLocalFile());
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
    return iconProvider->getFileIcon(absoluteFilePath());
}
