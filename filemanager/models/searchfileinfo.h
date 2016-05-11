#ifndef SEARCHFILEINFO_H
#define SEARCHFILEINFO_H

#include "fileinfo.h"

class SearchFileInfo : public AbstractFileInfo
{
public:
    SearchFileInfo();
    SearchFileInfo(const DUrl &url);

    bool exists() const Q_DECL_OVERRIDE;

    QString filePath() const Q_DECL_OVERRIDE;
    QString absoluteFilePath() const Q_DECL_OVERRIDE;
    QString fileName() const Q_DECL_OVERRIDE;
    QString displayName() const Q_DECL_OVERRIDE;

    QString path() const Q_DECL_OVERRIDE;
    QString absolutePath() const Q_DECL_OVERRIDE;

    bool isCanRename() const Q_DECL_OVERRIDE;
    bool isReadable() const Q_DECL_OVERRIDE;
    bool isWritable() const Q_DECL_OVERRIDE;
    bool isExecutable() const  Q_DECL_OVERRIDE;
    bool isHidden() const  Q_DECL_OVERRIDE;

    bool isFile() const Q_DECL_OVERRIDE;
    bool isDir() const Q_DECL_OVERRIDE;
    bool isSymLink() const  Q_DECL_OVERRIDE;

    QString owner() const  Q_DECL_OVERRIDE;
    uint ownerId() const  Q_DECL_OVERRIDE;
    QString group() const  Q_DECL_OVERRIDE;
    uint groupId() const  Q_DECL_OVERRIDE;

    qint64 size() const  Q_DECL_OVERRIDE;

    QDateTime created() const  Q_DECL_OVERRIDE;
    QDateTime lastModified() const  Q_DECL_OVERRIDE;
    QDateTime lastRead() const  Q_DECL_OVERRIDE;

    QString lastModifiedDisplayName() const  Q_DECL_OVERRIDE;
    QString createdDisplayName() const  Q_DECL_OVERRIDE;
    QString sizeDisplayName() const  Q_DECL_OVERRIDE;
    QString mimeTypeDisplayName() const  Q_DECL_OVERRIDE;

    QIcon fileIcon() const Q_DECL_OVERRIDE;
    DUrl parentUrl() const Q_DECL_OVERRIDE;

    quint8 supportViewMode() const Q_DECL_OVERRIDE;

    /// getFileInfoFun is get AbstractFileInfoPointer by index for caller
    int getIndexByFileInfo(getFileInfoFun fun, const AbstractFileInfoPointer &info, quint8 columnType,
                                   Qt::SortOrder order = Qt::AscendingOrder) const Q_DECL_OVERRIDE;

    quint8 userColumnCount() const Q_DECL_OVERRIDE;

    /// userColumnType = ColumnType::UserType + user column index
    QVariant userColumnDisplayName(quint8 userColumnType) const Q_DECL_OVERRIDE;

    /// get custom column data
    QVariant userColumnData(quint8 userColumnType) const Q_DECL_OVERRIDE;

    bool canRedirectionFileUrl() const Q_DECL_OVERRIDE;
    DUrl redirectedFileUrl() const Q_DECL_OVERRIDE;

private:
    DUrl m_parentUrl;
    AbstractFileInfoPointer realFileInfo;
};

#endif // SEARCHFILEINFO_H
