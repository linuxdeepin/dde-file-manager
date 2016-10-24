#ifndef TRASHFILEINFO_H
#define TRASHFILEINFO_H

#include "dabstractfileinfo.h"

class DFMEvent;
class TrashFileInfo : public DAbstractFileInfo
{
public:
    TrashFileInfo(const DUrl &url);

    bool isCanRename() const Q_DECL_OVERRIDE;
    bool isReadable() const Q_DECL_OVERRIDE;
    bool isWritable() const Q_DECL_OVERRIDE;
    bool isCanShare() const Q_DECL_OVERRIDE;

    QString fileDisplayName() const Q_DECL_OVERRIDE;

    void setUrl(const DUrl &fileUrl) Q_DECL_OVERRIDE;

    QFile::Permissions permissions() const Q_DECL_OVERRIDE;

    QVector<MenuAction> menuActionList(MenuType type) const Q_DECL_OVERRIDE;
    QSet<MenuAction> disableMenuActionList() const Q_DECL_OVERRIDE;

    QList<int> userColumnRoles() const Q_DECL_OVERRIDE;
    QVariant userColumnData(int userColumnRole) const Q_DECL_OVERRIDE;
    QVariant userColumnDisplayName(int userColumnRole) const Q_DECL_OVERRIDE;
    int userColumnWidth(int userColumnRole, const QFontMetrics &fontMetrics) const Q_DECL_OVERRIDE;

    sortFunction sortFunByColumn(int columnRole) const Q_DECL_OVERRIDE;
    bool columnDefaultVisibleForRole(int userColumnRole) const Q_DECL_OVERRIDE;

    MenuAction menuActionByColumnRole(int userColumnRole) const Q_DECL_OVERRIDE;

    bool canIteratorDir() const Q_DECL_OVERRIDE;
    bool makeAbsolute() Q_DECL_OVERRIDE;

    DUrl mimeDataUrl() const Q_DECL_OVERRIDE;
    Qt::DropActions supportedDragActions() const Q_DECL_OVERRIDE;
    Qt::DropActions supportedDropActions() const Q_DECL_OVERRIDE;

    QList<QIcon> additionalIcon() const Q_DECL_OVERRIDE;

    bool restore(const DFMEvent &event) const;
    QDateTime deletionDate() const;
    QString sourceFilePath() const;

private:
    QString desktopIconName;
    QString m_displayName;
    QString originalFilePath;
    QString displayDeletionDate;
    QDateTime m_deletionDate;

    void updateInfo();
};

#endif // TRASHFILEINFO_H
