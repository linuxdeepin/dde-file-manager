#include "fileinfo.h"
#include "../app/global.h"

#include <QDateTime>
#include <QDir>
#include <QMimeDatabase>

FileInfo::FileInfo()
{

}

FileInfo::FileInfo(const QString &file)
    : m_genuineFileInfo(file)
    , m_fileUrl(file)
{

}

FileInfo::FileInfo(const QFileInfo &fileInfo)
    : m_genuineFileInfo(fileInfo)
    , m_fileUrl(QUrl(fileInfo.absoluteFilePath()))
{

}

FileInfo::~FileInfo()
{

}

void FileInfo::setFile(const QString &file)
{
    m_genuineFileInfo.setFile(file);
    m_fileUrl = QUrl(file);
    m_mimeTypeName.clear();
}

bool FileInfo::exists() const
{
    if(!m_fileUrl.scheme().isEmpty() && m_fileUrl.path() == "/")
        return true;

    return m_genuineFileInfo.exists();
}

bool FileInfo::exists(const QString &file)
{
    QUrl url(file);

    if(!url.scheme().isEmpty() && url.path() == "/")
        return true;

    return QFileInfo::exists(file);
}

QMimeType FileInfo::mimeType(const QString &file)
{
    QMimeDatabase db;

    return db.mimeTypeForFile(file);
}

QString FileInfo::filePath() const
{
    if(!m_fileUrl.scheme().isEmpty() && m_fileUrl.path() == "/")
        return m_fileUrl.toString();

    return m_genuineFileInfo.filePath();
}

QString FileInfo::absoluteFilePath() const
{
    if(!m_fileUrl.scheme().isEmpty() && m_fileUrl.path() == "/")
        return m_fileUrl.toString();

    return m_genuineFileInfo.absoluteFilePath();
}

QString FileInfo::fileName() const
{
    if(!m_fileUrl.scheme().isEmpty() && m_fileUrl.path() == "/")
        return "/";

    return m_genuineFileInfo.fileName();
}

QString FileInfo::displayName() const
{
    return fileName();
}

QString FileInfo::path() const
{
    if(!m_fileUrl.scheme().isEmpty() && m_fileUrl.path() == "/")
        return "/";

    return m_genuineFileInfo.path();
}

QString FileInfo::absolutePath() const
{
    if(!m_fileUrl.scheme().isEmpty() && m_fileUrl.path() == "/")
        return "/";

    return m_genuineFileInfo.absolutePath();
}

bool FileInfo::isDesktopFile() const
{
    return m_genuineFileInfo.suffix() == DESKTOP_SURRIX;
}

bool FileInfo::isImageFile() const
{
    return false;
}

bool FileInfo::isCanRename() const
{
    if(!m_fileUrl.scheme().isEmpty() && m_fileUrl.path() == "/")
        return false;

    return QFileInfo(m_genuineFileInfo.absolutePath()).isWritable()
            && m_genuineFileInfo.isWritable();
}

bool FileInfo::isReadable() const
{
    if(!m_fileUrl.scheme().isEmpty() && m_fileUrl.path() == "/")
        return true;

    return m_genuineFileInfo.isReadable();
}

bool FileInfo::isWritable() const
{
    if(!m_fileUrl.scheme().isEmpty() && m_fileUrl.path() == "/")
        return false;

    return m_genuineFileInfo.isWritable();
}

bool FileInfo::isExecutable() const
{
    if(!m_fileUrl.scheme().isEmpty() && m_fileUrl.path() == "/")
        return false;

    return m_genuineFileInfo.isExecutable();
}

bool FileInfo::isHidden() const
{
    if(!m_fileUrl.scheme().isEmpty() && m_fileUrl.path() == "/")
        return false;

    return m_genuineFileInfo.isHidden();
}

bool FileInfo::isRelative() const
{
    if(!m_fileUrl.scheme().isEmpty() && m_fileUrl.path() == "/")
        return false;

    return m_genuineFileInfo.isRelative();
}

bool FileInfo::makeAbsolute()
{
    return m_genuineFileInfo.makeAbsolute();
}

bool FileInfo::isFile() const
{
    if(!m_fileUrl.scheme().isEmpty() && m_fileUrl.path() == "/")
        return false;

    return m_genuineFileInfo.isFile();
}

bool FileInfo::isDir() const
{
    if(!m_fileUrl.scheme().isEmpty() && m_fileUrl.path() == "/")
        return true;

    return m_genuineFileInfo.isDir();
}

bool FileInfo::isSymLink() const
{
    if(!m_fileUrl.scheme().isEmpty() && m_fileUrl.path() == "/")
        return false;

    return m_genuineFileInfo.isSymLink();
}

bool FileInfo::isRoot() const
{
    if(!m_fileUrl.scheme().isEmpty() && m_fileUrl.path() == "/")
        return false;

    return m_genuineFileInfo.isRoot();
}

QString FileInfo::readLink() const
{
    return m_genuineFileInfo.readLink();
}

QString FileInfo::owner() const
{
    return m_genuineFileInfo.owner();
}

uint FileInfo::ownerId() const
{
    return m_genuineFileInfo.ownerId();
}

QString FileInfo::group() const
{
    return m_genuineFileInfo.group();
}

uint FileInfo::groupId() const
{
    return m_genuineFileInfo.groupId();
}

bool FileInfo::permission(QFileDevice::Permissions permissions) const
{
    return m_genuineFileInfo.permission(permissions);
}

QFileDevice::Permissions FileInfo::permissions() const
{
    return m_genuineFileInfo.permissions();
}

qint64 FileInfo::size() const
{
    return m_genuineFileInfo.size();
}

QDateTime FileInfo::created() const
{
    return m_genuineFileInfo.created();
}

QDateTime FileInfo::lastModified() const
{
    return m_genuineFileInfo.lastModified();
}

QDateTime FileInfo::lastRead() const
{
    return m_genuineFileInfo.lastRead();
}

QString FileInfo::mimeTypeName() const
{
    if(m_mimeTypeName.isNull())
        m_mimeTypeName = mimeType(absoluteFilePath()).name();

    return m_mimeTypeName;
}

const QFileInfo &FileInfo::genuineFileInfo() const
{
    return m_genuineFileInfo;
}

QT_BEGIN_NAMESPACE
QDebug operator<<(QDebug deg, const FileInfo &info)
{
    deg << "file path:" << info.absoluteFilePath()
        << "file size:" << info.size() << "mime type:"
        << info.mimeTypeName();

    return deg;
}
QT_END_NAMESPACE
