#ifndef DESKTOPFILEINFO_H
#define DESKTOPFILEINFO_H

#include "dfileinfo.h"

#include <QObject>

#define DESKTOP_SURRIX "desktop"

class DesktopFileInfoPrivate;
class DesktopFileInfo : public DFileInfo
{
public:
    DesktopFileInfo(const DUrl & fileUrl);
    DesktopFileInfo(const QFileInfo &fileInfo);

    ~DesktopFileInfo();

    QString getName() const;
    QString getExec() const;
    QString getIconName() const;
    QString getType() const;
    QStringList getCategories() const;

    QIcon fileIcon() const Q_DECL_OVERRIDE;
    QString fileDisplayName() const Q_DECL_OVERRIDE;

    void refresh() Q_DECL_OVERRIDE;

    QString iconName() const Q_DECL_OVERRIDE;
    QString genericIconName() const Q_DECL_OVERRIDE;

    static QMap<QString, QVariant> getDesktopFileInfo(const DUrl &fileUrl);

    QVector<MenuAction> menuActionList(MenuType type = SingleFile) const Q_DECL_OVERRIDE;
    QSet<MenuAction> disableMenuActionList() const Q_DECL_OVERRIDE;
    QList<QIcon> additionalIcon() const;
    Qt::DropActions supportedDragActions() const Q_DECL_OVERRIDE;

    bool canDrop() const Q_DECL_OVERRIDE;

    static DUrl trashDesktopFileUrl();
    static DUrl computerDesktopFileUrl();

private:
    Q_DECLARE_PRIVATE(DesktopFileInfo)
};

#endif // DESKTOPFILEINFO_H
