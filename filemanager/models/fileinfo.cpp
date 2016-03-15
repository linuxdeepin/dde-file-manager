#include "fileinfo.h"

#include <QDateTime>
#include <QDir>
#include <QMimeDatabase>

FileInfo::FileInfo()
{

}

FileInfo::FileInfo(const QString &file) :
    m_fileInfo(QFileInfo(file))
{
    m_mimeTypeName = mimeType(file).name();
}

FileInfo::FileInfo(const QFileInfo &fileInfo) :
    m_fileInfo(fileInfo)
{
    m_mimeTypeName = mimeType(fileInfo.absoluteFilePath()).name();
}

void FileInfo::setFile(const QString &file)
{
    m_fileInfo.setFile(file);
    m_mimeTypeName = mimeType(file).name();
}

bool FileInfo::exists() const
{
    return m_fileInfo.exists();
}

bool FileInfo::exists(const QString &file)
{
    return QFileInfo::exists(file);
}

QMimeType FileInfo::mimeType(const QString &file)
{
    QMimeDatabase db;

    return db.mimeTypeForFile(file);
}

QString FileInfo::filePath() const
{
    return m_fileInfo.filePath();
}

QString FileInfo::absoluteFilePath() const
{
    return m_fileInfo.absoluteFilePath();
}

QString FileInfo::fileName() const
{
    return m_fileInfo.fileName();
}

QString FileInfo::path() const
{
    return m_fileInfo.path();
}

QString FileInfo::absolutePath() const
{
    return m_fileInfo.absolutePath();
}

bool FileInfo::isCanRename() const
{
    if(m_fileInfo.isDir()) {
        QDir dir = m_fileInfo.absoluteDir();

        return dir.cdUp() ? QFileInfo(dir.absolutePath()).isWritable()
                          : m_fileInfo.isWritable();
    }

    return QFileInfo(m_fileInfo.absolutePath()).isWritable();
}

bool FileInfo::isReadable() const
{
    return m_fileInfo.isReadable();
}

bool FileInfo::isWritable() const
{
    return m_fileInfo.isWritable();
}

bool FileInfo::isExecutable() const
{
    return m_fileInfo.isExecutable();
}

bool FileInfo::isHidden() const
{
    return m_fileInfo.isHidden();
}

bool FileInfo::isRelative() const
{
    return m_fileInfo.isRelative();
}

bool FileInfo::makeAbsolute()
{
    return m_fileInfo.makeAbsolute();
}

bool FileInfo::isFile() const
{
    return m_fileInfo.isFile();
}

bool FileInfo::isDir() const
{
    return m_fileInfo.isDir();
}

bool FileInfo::isSymLink() const
{
    return m_fileInfo.isSymLink();
}

bool FileInfo::isRoot() const
{
    return m_fileInfo.isRoot();
}

QString FileInfo::readLink() const
{
    return m_fileInfo.readLink();
}

QString FileInfo::owner() const
{
    return m_fileInfo.owner();
}

uint FileInfo::ownerId() const
{
    return m_fileInfo.ownerId();
}

QString FileInfo::group() const
{
    return m_fileInfo.group();
}

uint FileInfo::groupId() const
{
    return m_fileInfo.groupId();
}

bool FileInfo::permission(QFileDevice::Permissions permissions) const
{
    return m_fileInfo.permission(permissions);
}

QFileDevice::Permissions FileInfo::permissions() const
{
    return m_fileInfo.permissions();
}

qint64 FileInfo::size() const
{
    return m_fileInfo.size();
}

QDateTime FileInfo::created() const
{
    return m_fileInfo.created();
}

QDateTime FileInfo::lastModified() const
{
    return m_fileInfo.lastModified();
}

QDateTime FileInfo::lastRead() const
{
    return m_fileInfo.lastRead();
}

QString FileInfo::mimeTypeName() const
{
    return m_mimeTypeName;
}
