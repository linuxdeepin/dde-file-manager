#ifndef AVFSFILEINFO_H
#define AVFSFILEINFO_H

#include "interfaces/dabstractfileinfo.h"

class AVFSFileInfoPrivate;
class AVFSFileInfo : public DAbstractFileInfo
{
public:
    AVFSFileInfo(const QString &archRootPath, const DUrl& avfsUrl);

    bool isCanRename() const Q_DECL_OVERRIDE;
    bool isWritable() const Q_DECL_OVERRIDE;
    bool isCanShare() const Q_DECL_OVERRIDE;
    bool canIteratorDir() const Q_DECL_OVERRIDE;
    bool isDir() const Q_DECL_OVERRIDE;

    QVector<MenuAction> menuActionList(MenuType type) const Q_DECL_OVERRIDE;

    static DUrl realFileUrl(QString archRootPath, const DUrl& avfsUrl);
protected:
    explicit AVFSFileInfo(AVFSFileInfoPrivate &dd);

    Q_DECLARE_PRIVATE(AVFSFileInfo)
};

#endif // AVFSFILEINFO_H
