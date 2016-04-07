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

FileInfo::FileInfo(const QString &file)
    : AbstractFileInfo()
{
    QUrl url(file);

    if(url.scheme().isEmpty())
        url = QUrl::fromLocalFile(file);

    AbstractFileInfo::setUrl(url);
}

FileInfo::FileInfo(const QFileInfo &fileInfo)
    : AbstractFileInfo()
{
    AbstractFileInfo::setUrl(QUrl::fromLocalFile(fileInfo.absoluteFilePath()));
}

void FileInfo::setFile(const QString &file)
{
    QUrl url(file);

    if(url.scheme().isEmpty())
        url = QUrl::fromLocalFile(file);

    setUrl(url);
}

bool FileInfo::exists(const QString &file)
{
    QUrl url(file);

    if(url.scheme().isEmpty())
        url = QUrl::fromLocalFile(file);

    return QFileInfo::exists(url.toLocalFile());
}

QMimeType FileInfo::mimeType(const QString &file)
{
    QMimeDatabase db;

    return db.mimeTypeForFile(file);
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
