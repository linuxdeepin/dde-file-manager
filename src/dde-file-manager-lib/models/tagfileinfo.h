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

    virtual int filesCount() const override;
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

    /**
     * @brief canDragCompress 是否支持拖拽压缩
     * @return
     */
    bool canDragCompress() const override;
};



#endif // TAGFILEINFO_H
