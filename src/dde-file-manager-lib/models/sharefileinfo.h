/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     yanghao<yanghao@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
 *             hujianzhong<hujianzhong@uniontech.com>
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

#ifndef SHAREFILEINFO_H
#define SHAREFILEINFO_H

#include <QObject>
#include "dabstractfileinfo.h"

class ShareFileInfo : public DAbstractFileInfo
{
public:
    explicit ShareFileInfo(const DUrl &url);
    ~ShareFileInfo() override;
    bool exists() const override;
    bool isDir() const override;
    bool canRename() const override;
    bool isReadable() const override;
    bool isWritable() const override;

    QString fileDisplayName() const override;

    QVector<MenuAction> menuActionList(MenuType type) const override;
    QSet<MenuAction> disableMenuActionList() const override;

    bool columnDefaultVisibleForRole(int userColumnRole) const override;

    MenuAction menuActionByColumnRole(int userColumnRole) const override;

    bool canIteratorDir() const override;
    bool makeAbsolute() override;

    DUrl mimeDataUrl() const override;
    DUrl parentUrl() const override;

    bool isShared() const override;
    bool canTag() const override;
    bool isVirtualEntry() const override;
    virtual bool canDrop() const override;

    Qt::ItemFlags fileItemDisableFlags() const override;
    QList<QIcon> additionalIcon() const override;

    virtual bool canRedirectionFileUrl() const override;
    virtual DUrl redirectedFileUrl() const override;

private:
    QString m_displayName;

};

#endif // SHAREFILEINFO_H
