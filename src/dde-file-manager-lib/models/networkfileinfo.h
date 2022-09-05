// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef NETWORKFILEINFO_H
#define NETWORKFILEINFO_H


#include "dfileinfo.h"
#include "gvfs/networkmanager.h"


class NetworkFileInfo: public DAbstractFileInfo
{
public:
    explicit NetworkFileInfo(const DUrl &url);
    ~NetworkFileInfo() override;

    QString filePath() const override;
    QString absoluteFilePath() const override;
    bool exists() const override;

    bool isReadable() const override;
    bool isWritable() const override;
    bool isVirtualEntry() const override;
    virtual bool canDrop() const override;

    bool canRename() const override;
    bool canIteratorDir() const override;
//    QIcon fileIcon() const override;
    bool isDir() const override;
    DUrl parentUrl() const override;
    QString fileDisplayName() const override;

//    QString mimeTypeName(DMimeDatabase::MatchMode mode = QMimeDatabase::MatchDefault) const override;

    NetworkNode networkNode() const;
    void setNetworkNode(const NetworkNode &networkNode);

    QVector<MenuAction> menuActionList(MenuType type) const override;

    QList<SelectionMode> supportSelectionModes() const override;
    Qt::ItemFlags fileItemDisableFlags() const override;

    quint8 supportViewMode() const override;

    int filesCount() const override;

    QString iconName() const override;

    bool canRedirectionFileUrl() const override;
    DUrl redirectedFileUrl() const override;

private:
    NetworkNode m_networkNode;
    mutable QString mountPoint;
};

#endif // NETWORKFILEINFO_H
