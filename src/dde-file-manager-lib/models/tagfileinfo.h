#ifndef TAGFILEINFO_H
#define TAGFILEINFO_H

#include <QObject>

#include "dabstractfileinfo.h"

class TagFileInfo : public DAbstractFileInfo
{
public:
    explicit TagFileInfo(const DUrl &url);
    virtual ~TagFileInfo() = default;

    virtual bool isDir() const override;
    virtual bool makeAbsolute() override;
    virtual bool exists() const override;

    bool canRename() const override;

    virtual bool isTaged() const override;
    virtual bool isWritable() const override;
    virtual bool canRedirectionFileUrl() const override;
//    virtual bool columnDefaultVisibleForRole(int role) const override;

    QFileDevice::Permissions permissions() const override;

    virtual DUrl parentUrl() const override;
    virtual QString iconName() const override;
    virtual DUrl goToUrlWhenDeleted() const override;
    virtual DUrl redirectedFileUrl() const override;
    virtual Qt::ItemFlags fileItemDisableFlags() const override;
    virtual QVector<MenuAction> menuActionList(MenuType type) const override;

    DUrl getUrlByNewFileName(const QString &name) const override;

    bool canIteratorDir() const override;

    QVariantHash extraProperties() const override;

    QList<int> userColumnRoles() const override;

    DUrl mimeDataUrl() const override;
    Qt::DropActions supportedDragActions() const override;
    Qt::DropActions supportedDropActions() const override;
    bool canDrop() const override;
    bool isVirtualEntry() const override;

    /**
     * @brief sizeDisplayName 列表模式下，获取大小
     * @return
     */
    QString sizeDisplayName() const override;
};



#endif // TAGFILEINFO_H
