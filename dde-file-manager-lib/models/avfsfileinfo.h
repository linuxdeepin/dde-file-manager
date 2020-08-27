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

#ifndef AVFSFILEINFO_H
#define AVFSFILEINFO_H

#include "interfaces/dabstractfileinfo.h"

class AVFSFileInfoPrivate;
class AVFSFileInfo : public DAbstractFileInfo
{
public:
    AVFSFileInfo(const DUrl &avfsUrl);

    bool canRename() const Q_DECL_OVERRIDE;
    bool isWritable() const Q_DECL_OVERRIDE;
    bool canShare() const Q_DECL_OVERRIDE;
    bool canIteratorDir() const Q_DECL_OVERRIDE;
    bool isDir() const Q_DECL_OVERRIDE;
    bool canManageAuth() const Q_DECL_OVERRIDE;

    QString toLocalFile() const override;
    DUrl parentUrl() const override;
    QVector<MenuAction> menuActionList(MenuType type) const Q_DECL_OVERRIDE;

    static DUrl realFileUrl(const DUrl &avfsUrl);
    static DUrl realDirUrl(const DUrl &avfsUrl);
protected:
    explicit AVFSFileInfo(AVFSFileInfoPrivate &dd);

    Q_DECLARE_PRIVATE(AVFSFileInfo)
};

#endif // AVFSFILEINFO_H
