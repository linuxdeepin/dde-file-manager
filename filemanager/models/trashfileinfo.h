#ifndef TRASHFILEINFO_H
#define TRASHFILEINFO_H

#include "abstractfileinfo.h"
#include "../app/fmevent.h"

class TrashFileInfo : public AbstractFileInfo
{
public:
    TrashFileInfo();
    TrashFileInfo(const DUrl &url);
    TrashFileInfo(const QString &url);

    bool isCanRename() const Q_DECL_OVERRIDE;
    bool isWritable() const Q_DECL_OVERRIDE;

    QString displayName() const;

    void setUrl(const DUrl &fileUrl) Q_DECL_OVERRIDE;

    QIcon fileIcon() const Q_DECL_OVERRIDE;

    QString mimeTypeName() const Q_DECL_OVERRIDE;

    QFile::Permissions permissions() const Q_DECL_OVERRIDE;

    QVector<MenuAction> menuActionList(MenuType type) const Q_DECL_OVERRIDE;

    bool restore(const FMEvent &event) const;

private:
    QString desktopIconName;
    QString m_displayName;
    QString originalFilePath;
    QString deletionDate;

    void updateInfo();
};

#endif // TRASHFILEINFO_H
