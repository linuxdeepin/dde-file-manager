#ifndef SEARCHFILEINFO_H
#define SEARCHFILEINFO_H

#include "fileinfo.h"

class SearchFileInfo : public FileInfo
{
public:
    SearchFileInfo();
    SearchFileInfo(const DUrl &url);

    bool isCanRename() const Q_DECL_OVERRIDE;
    bool isReadable() const Q_DECL_OVERRIDE;
    bool isWritable() const Q_DECL_OVERRIDE;
    bool isDir() const Q_DECL_OVERRIDE;

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
};

#endif // SEARCHFILEINFO_H
