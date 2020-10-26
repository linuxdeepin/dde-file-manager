/*
 * Copyright (C) 2016 ~ 2018 Deepin Technology Co., Ltd.
 *               2016 ~ 2018 dragondjf
 *
 * Author:     dragondjf<dingjiangfeng@deepin.com>
 *
 * Maintainer: dragondjf<dingjiangfeng@deepin.com>
 *             zccrs<zhangjide@deepin.com>
 *             Tangtong<tangtong@deepin.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef NETWORKFILEINFO_H
#define NETWORKFILEINFO_H


#include "dfileinfo.h"
#include "gvfs/networkmanager.h"


class NetworkFileInfo: public DAbstractFileInfo
{
public:
    explicit NetworkFileInfo(const DUrl &url);
    ~NetworkFileInfo();

    QString filePath() const Q_DECL_OVERRIDE;
    QString absoluteFilePath() const Q_DECL_OVERRIDE;
    bool exists() const Q_DECL_OVERRIDE;

    bool isReadable() const Q_DECL_OVERRIDE;
    bool isWritable() const Q_DECL_OVERRIDE;
    bool isVirtualEntry() const Q_DECL_OVERRIDE;
    virtual bool canDrop() const Q_DECL_OVERRIDE;

    bool canRename() const Q_DECL_OVERRIDE;
    bool canIteratorDir() const override;
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

    int filesCount() const override;

    QString iconName() const Q_DECL_OVERRIDE;

    bool canRedirectionFileUrl() const override;
    DUrl redirectedFileUrl() const override;

private:
    NetworkNode m_networkNode;
    mutable QString mountPoint;
};

#endif // NETWORKFILEINFO_H
