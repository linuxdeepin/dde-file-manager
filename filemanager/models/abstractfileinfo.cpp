#include "abstractfileinfo.h"

#include <QDateTime>
#include <QDebug>

AbstractFileInfo::AbstractFileInfo()
    : data(new FileInfoData)
{

}

AbstractFileInfo::AbstractFileInfo(const QUrl &url)
    : data(new FileInfoData)
{
    data->url = url.toString();
    data->fileInfo.setFile(url.path());
}

AbstractFileInfo::AbstractFileInfo(const QString &url)
    : data(new FileInfoData)
{
    data->url = url;
    data->fileInfo.setFile(QUrl(url).path());
}

AbstractFileInfo::AbstractFileInfo(const AbstractFileInfo &other)
    : data(other.data)
{

}

AbstractFileInfo::~AbstractFileInfo()
{

}

void AbstractFileInfo::setUrl(const QString &url)
{
    data->url = url;
    data->fileInfo.setFile(QUrl(url).path());
}

void AbstractFileInfo::setUrl(const QUrl &url)
{
    data->url = url.toString();
    data->fileInfo.setFile(url.path());
}

bool AbstractFileInfo::exists() const
{
    return data->fileInfo.exists();
}

QString AbstractFileInfo::filePath() const
{
    return data->fileInfo.filePath();
}

QString AbstractFileInfo::absoluteFilePath() const
{
    return data->fileInfo.absoluteFilePath();
}

QString AbstractFileInfo::fileName() const
{
    return data->fileInfo.fileName();
}

QString AbstractFileInfo::displayName() const
{
    return fileName();
}

QString AbstractFileInfo::path() const
{
    return data->fileInfo.path();
}

QString AbstractFileInfo::absolutePath() const
{
    return data->fileInfo.absolutePath();
}

bool AbstractFileInfo::isReadable() const
{
    return data->fileInfo.isReadable();
}

bool AbstractFileInfo::isWritable() const
{
    return data->fileInfo.isWritable();
}

bool AbstractFileInfo::isExecutable() const
{
    return data->fileInfo.isExecutable();
}

bool AbstractFileInfo::isHidden() const
{
    return data->fileInfo.isHidden();
}

bool AbstractFileInfo::isRelative() const
{
    return data->fileInfo.isRelative();
}

bool AbstractFileInfo::isAbsolute() const
{
    return data->fileInfo.isAbsolute();
}

bool AbstractFileInfo::makeAbsolute()
{
    bool ok = data->fileInfo.makeAbsolute();

    QUrl url(data->url);

    url.setPath(data->fileInfo.filePath());
    data->url = url.toString();

    return ok;
}

bool AbstractFileInfo::isFile() const
{
    return data->fileInfo.isFile();
}

bool AbstractFileInfo::isDir() const
{
    return data->fileInfo.isDir();
}

bool AbstractFileInfo::isSymLink() const
{
    return data->fileInfo.isSymLink();
}

QString AbstractFileInfo::readLink() const
{
    return data->fileInfo.readLink();
}

QString AbstractFileInfo::owner() const
{
    return data->fileInfo.owner();
}

uint AbstractFileInfo::ownerId() const
{
    return data->fileInfo.ownerId();
}

QString AbstractFileInfo::group() const
{
    return data->fileInfo.group();
}

uint AbstractFileInfo::groupId() const
{
    return data->fileInfo.groupId();
}

QFileDevice::Permissions AbstractFileInfo::permissions() const
{
    return data->fileInfo.permissions();
}

qint64 AbstractFileInfo::size() const
{
    return data->fileInfo.size();
}

QDateTime AbstractFileInfo::created() const
{
    return data->fileInfo.created();
}

QDateTime AbstractFileInfo::lastModified() const
{
    return data->fileInfo.lastModified();
}

QDateTime AbstractFileInfo::lastRead() const
{
    return data->fileInfo.lastRead();
}

QString AbstractFileInfo::parentUrl() const
{
    return scheme() + "://" + absolutePath();
}

QT_BEGIN_NAMESPACE
QDebug operator<<(QDebug deg, const AbstractFileInfo &info)
{
    deg << "file url:" << info.fileUrl()
        << "mime type:" << info.mimeTypeName();

    return deg;
}
QT_END_NAMESPACE
