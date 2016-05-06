#ifndef RECENTFILEINFO_H
#define RECENTFILEINFO_H

#include "abstractfileinfo.h"
#include <QDateTime>

class RecentFileInfo : public AbstractFileInfo
{
public:
    RecentFileInfo();
    RecentFileInfo(const DUrl &url);
    RecentFileInfo(const QString &url);

    bool isCanRename() const Q_DECL_OVERRIDE;
    bool isWritable() const Q_DECL_OVERRIDE;
    bool isReadable() const Q_DECL_OVERRIDE;

    bool isDir() const Q_DECL_OVERRIDE;

    QIcon fileIcon() const Q_DECL_OVERRIDE;

    QVector<MenuAction> menuActionList(MenuType type) const Q_DECL_OVERRIDE;

    DUrl parentUrl() const Q_DECL_OVERRIDE;



    QString lastOpenedDisplayName() const;
    QDateTime lastOpened() const;
    void setLastOpened(const QDateTime &lastOpened);

private:
    QString desktopIconName;
    QDateTime m_lastOpened;
};

#endif // RECENTFILEINFO_H
