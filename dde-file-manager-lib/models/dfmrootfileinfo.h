#ifndef DFMROOTFILEINFO_H
#define DFMROOTFILEINFO_H

#include "dfileinfo.h"

class DFMRootFileInfoPrivate;

class DFMRootFileInfo : public DAbstractFileInfo
{
public:
    DFMRootFileInfo(const DUrl &url);
    bool exists() const override;

    QString suffix() const override;
    QString fileDisplayName() const override;

    bool canRename() const override;
    bool canShare() const override;
    bool canFetch() const override;
    bool isReadable() const override;
    bool isWritable() const override;
    bool isExecutable() const override;
    bool isHidden() const override;
    bool isRelative() const override;
    bool isAbsolute() const override;
    bool isShared() const override;
    bool isTaged() const override;
    bool isWritableShared() const override;
    bool isAllowGuestShared() const override;

    bool isFile() const override;
    bool isDir() const override;
    int filesCount() const override;
    QString iconName() const override;

    QVector<MenuAction> menuActionList(MenuType type = SingleFile) const override;

    bool canRedirectionFileUrl() const override;
    DUrl redirectedFileUrl() const override;
    bool canDrop() const override;
    Qt::DropActions supportedDragActions() const override;
    Qt::DropActions supportedDropActions() const override;

    QVariantHash extraProperties() const override;

    void checkCache();
private:
    QScopedPointer<DFMRootFileInfoPrivate> d_ptr;
    Q_DECLARE_PRIVATE(DFMRootFileInfo)
};

#endif // DFMROOTFILEINFO_H
