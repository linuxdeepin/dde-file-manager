#ifndef FILEINFO_H
#define FILEINFO_H

#include <QObject>
#include <QFileInfo>
#include <QMimeType>

#include "utils/debugobejct.h"

class FileInfo
{
public:
    FileInfo();
    FileInfo(const QString & file);
    FileInfo(const QFileInfo &fileInfo);

    void setFile(const QString &file);

    bool exists() const;
    static bool exists(const QString &file);
    static QMimeType mimeType(const QString &file);

    QString filePath() const;
    QString absoluteFilePath() const;
    QString fileName() const;

    QString path() const;
    QString absolutePath() const;

    bool isCanRename() const;
    bool isReadable() const;
    bool isWritable() const;
    bool isExecutable() const;
    bool isHidden() const;

    bool isRelative() const;
    inline bool isAbsolute() const;
    bool makeAbsolute();

    bool isFile() const;
    bool isDir() const;
    bool isSymLink() const;
    bool isRoot() const;

    QString readLink() const;
    inline QString symLinkTarget() const { return readLink(); }

    QString owner() const;
    uint ownerId() const;
    QString group() const;
    uint groupId() const;

    bool permission(QFile::Permissions permissions) const;
    QFile::Permissions permissions() const;

    qint64 size() const;

    QDateTime created() const;
    QDateTime lastModified() const;
    QDateTime lastRead() const;

    QString mimeTypeName() const;

private:
    QFileInfo m_fileInfo;
    QString m_mimeTypeName;
};

typedef QList<FileInfo> FileInfoList;

Q_DECLARE_METATYPE(FileInfo)

#endif // FILEINFO_H
