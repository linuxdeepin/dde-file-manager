/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     luzhen<luzhen@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             luzhen<luzhen@uniontech.com>
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

#pragma once

#include "dabstractfileinfo.h"
#include <QIcon>



class VaultFileInfoPrivate;
class VaultFileInfo : public DAbstractFileInfo
{
public:
    explicit VaultFileInfo(const DUrl &url);
    ~VaultFileInfo() override;

    bool exists() const override;

    DUrl parentUrl() const override;
    QString iconName() const override;
    QString genericIconName() const override;

    DUrl mimeDataUrl() const override;

    bool canRedirectionFileUrl() const override;
    DUrl redirectedFileUrl() const override;
    bool canIteratorDir() const override;

    QString subtitleForEmptyFloder() const override;

    DUrl getUrlByNewFileName(const QString &fileName) const override;

    QList<QIcon> additionalIcon() const override;
    bool isWritable() const override;
    bool isSymLink() const override;

    QFile::Permissions permissions() const override;
    QSet<MenuAction> disableMenuActionList() const override;

    // 右键菜单
    QVector<MenuAction> menuActionList(MenuType type = SingleFile) const override;
    QMap<MenuAction, QVector<MenuAction> > subMenuActionList(MenuType type = SingleFile) const override;

    bool canRename() const override;
    bool canShare() const override;
    bool canTag() const override;
    QIcon fileIcon() const override;
    QString fileDisplayName() const override;
    qint64 size() const override;
    bool isDir() const override;
    bool canDrop() const override;
    // 是否支持拖拽压缩
    bool canDragCompress() const override;

    /**
     * @brief isAncestorsUrl 拆分路径
     * @param url            文件路径信息
     * @param ancestors      拆分后的路径列表
     * @return               拆分是否成功
     */
    bool isAncestorsUrl(const DUrl &url, QList<DUrl> *ancestors) const override;

private:
    bool isRootDirectory() const;

    Q_DECLARE_PRIVATE(VaultFileInfo)
};
