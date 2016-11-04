#ifndef FILEINFO_H
#define FILEINFO_H

#include "dabstractfileinfo.h"

class DFileInfoPrivate;
class DFileInfo : public DAbstractFileInfo
{
public:
    explicit DFileInfo(const QString& filePath);
    explicit DFileInfo(const DUrl& fileUrl);
    explicit DFileInfo(const QFileInfo &fileInfo);

    static bool exists(const DUrl &fileUrl);
    static QMimeType mimeType(const QString &filePath);

    bool exists() const Q_DECL_OVERRIDE;

    QString path() const Q_DECL_OVERRIDE;
    QString filePath() const Q_DECL_OVERRIDE;
    QString absolutePath() const Q_DECL_OVERRIDE;
    QString absoluteFilePath() const Q_DECL_OVERRIDE;
    QString fileName() const Q_DECL_OVERRIDE;

    bool isCanRename() const Q_DECL_OVERRIDE;
    bool isCanShare() const Q_DECL_OVERRIDE;

    bool isReadable() const Q_DECL_OVERRIDE;
    bool isWritable() const Q_DECL_OVERRIDE;
    bool isExecutable() const Q_DECL_OVERRIDE;
    bool isHidden() const Q_DECL_OVERRIDE;
    bool isRelative() const Q_DECL_OVERRIDE;
    bool isAbsolute() const Q_DECL_OVERRIDE;
    bool isShared() const Q_DECL_OVERRIDE;
    bool makeAbsolute() Q_DECL_OVERRIDE;

    bool isFile() const Q_DECL_OVERRIDE;
    bool isDir() const Q_DECL_OVERRIDE;
    bool isSymLink() const Q_DECL_OVERRIDE;

    DUrl symLinkTarget() const Q_DECL_OVERRIDE;

    QString owner() const Q_DECL_OVERRIDE;
    uint ownerId() const Q_DECL_OVERRIDE;
    QString group() const Q_DECL_OVERRIDE;
    uint groupId() const Q_DECL_OVERRIDE;

    bool permission(QFile::Permissions permissions) const Q_DECL_OVERRIDE;
    QFile::Permissions permissions() const Q_DECL_OVERRIDE;

    qint64 size() const Q_DECL_OVERRIDE;
    int filesCount() const Q_DECL_OVERRIDE;

    QDateTime created() const Q_DECL_OVERRIDE;
    QDateTime lastModified() const Q_DECL_OVERRIDE;
    QDateTime lastRead() const Q_DECL_OVERRIDE;

    QMimeType mimeType() const Q_DECL_OVERRIDE;

    bool canIteratorDir() const Q_DECL_OVERRIDE;

    QString subtitleForEmptyFloder() const Q_DECL_OVERRIDE;

    QString fileDisplayName() const Q_DECL_OVERRIDE;

    DUrl goToUrl() const Q_DECL_OVERRIDE;

protected:
    explicit DFileInfo(DFileInfoPrivate &dd);

    Q_DECLARE_PRIVATE(DFileInfo)
};

#endif // FILEINFO_H
