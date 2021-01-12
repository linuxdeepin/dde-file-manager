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
