#ifndef NETWORKFILEINFO_H
#define NETWORKFILEINFO_H


#include "dfileinfo.h"
#include "gvfs/networkmanager.h"


class NetworkFileInfo: public DAbstractFileInfo
{
public:
    NetworkFileInfo(const DUrl &url);
    ~NetworkFileInfo();

    QString filePath() const Q_DECL_OVERRIDE;
    QString absoluteFilePath() const Q_DECL_OVERRIDE;
    bool exists() const Q_DECL_OVERRIDE;

    bool isReadable() const Q_DECL_OVERRIDE;
    bool isWritable() const Q_DECL_OVERRIDE;

    bool canRename() const Q_DECL_OVERRIDE;
//    QIcon fileIcon() const Q_DECL_OVERRIDE;
    bool isDir() const Q_DECL_OVERRIDE;
    DUrl parentUrl() const Q_DECL_OVERRIDE;
    QString fileDisplayName() const Q_DECL_OVERRIDE;

//    QString mimeTypeName(DMimeDatabase::MatchMode mode = QMimeDatabase::MatchDefault) const Q_DECL_OVERRIDE;

    NetworkNode networkNode() const;
    void setNetworkNode(const NetworkNode& networkNode);

    QVector<MenuAction> menuActionList(MenuType type) const Q_DECL_OVERRIDE;

    QList<SelectionMode> supportSelectionModes() const Q_DECL_OVERRIDE;
    Qt::ItemFlags fileItemDisableFlags() const Q_DECL_OVERRIDE;

    quint8 supportViewMode() const Q_DECL_OVERRIDE;

    QString iconName() const Q_DECL_OVERRIDE;

private:
    NetworkNode m_networkNode;
};

#endif // NETWORKFILEINFO_H
