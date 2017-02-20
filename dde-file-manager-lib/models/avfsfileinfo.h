#ifndef AVFSFILEINFO_H
#define AVFSFILEINFO_H

#include "interfaces/dabstractfileinfo.h"

class AVFSFileInfoPrivate;
class AVFSFileInfo : public DAbstractFileInfo
{
public:
    AVFSFileInfo(const DUrl& avfsUrl);

    bool canRename() const Q_DECL_OVERRIDE;
    bool isWritable() const Q_DECL_OVERRIDE;
    bool canShare() const Q_DECL_OVERRIDE;
    bool canIteratorDir() const Q_DECL_OVERRIDE;
    bool isDir() const Q_DECL_OVERRIDE;

    QVector<MenuAction> menuActionList(MenuType type) const Q_DECL_OVERRIDE;

    static DUrl realFileUrl(const DUrl& avfsUrl);
    static DUrl realDirUrl(const DUrl& avfsUrl);
protected:
    explicit AVFSFileInfo(AVFSFileInfoPrivate &dd);

    Q_DECLARE_PRIVATE(AVFSFileInfo)
};

#endif // AVFSFILEINFO_H
