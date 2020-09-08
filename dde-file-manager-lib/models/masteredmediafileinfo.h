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
#ifndef MASTEREDMEDIAFILEINFO_H
#define MASTEREDMEDIAFILEINFO_H

#include "dfileinfo.h"

class MasteredMediaFileInfo : public DAbstractFileInfo
{
public:
    MasteredMediaFileInfo(const DUrl &url);

    bool exists() const Q_DECL_OVERRIDE;
    bool isReadable() const Q_DECL_OVERRIDE;
    bool isWritable() const Q_DECL_OVERRIDE;
    bool isDir() const Q_DECL_OVERRIDE;

    int filesCount() const Q_DECL_OVERRIDE;
    QString fileDisplayName() const Q_DECL_OVERRIDE;

    QVariantHash extraProperties() const Q_DECL_OVERRIDE;
    QVector<MenuAction> menuActionList(MenuType type) const Q_DECL_OVERRIDE;

    bool canRedirectionFileUrl() const Q_DECL_OVERRIDE;
    DUrl redirectedFileUrl() const Q_DECL_OVERRIDE;
    DUrl mimeDataUrl() const Q_DECL_OVERRIDE;

    bool canIteratorDir() const Q_DECL_OVERRIDE;
    DUrl parentUrl() const Q_DECL_OVERRIDE;
    DUrl goToUrlWhenDeleted() const Q_DECL_OVERRIDE;
    QString toLocalFile() const Q_DECL_OVERRIDE;

    bool canDrop() const Q_DECL_OVERRIDE;
    bool canTag() const Q_DECL_OVERRIDE;
    bool canRename() const Q_DECL_OVERRIDE;
    QSet<MenuAction> disableMenuActionList() const Q_DECL_OVERRIDE;
    void refresh(const bool isForce = false) Q_DECL_OVERRIDE;

private:
    DUrl m_backerUrl;
};

#endif // MASTEREDMEDIAFILEINFO_H
