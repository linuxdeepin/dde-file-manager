/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     yanghao<yanghao@uniontech.com>
 *
 * Maintainer: liuyangming<liuyangming@uniontech.com>
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
#ifndef TRASHFILEINFO_H
#define TRASHFILEINFO_H

#include "dfmplugin_trash_global.h"
#include "dfm-base/interfaces/abstractfileinfo.h"

DPTRASH_BEGIN_NAMESPACE

class TrashFileInfoPrivate;
class TrashFileInfo : public DFMBASE_NAMESPACE::AbstractFileInfo
{
    Q_GADGET
    friend class TrashFileInfoPrivate;

public:
    explicit TrashFileInfo(const QUrl &url);
    ~TrashFileInfo() override;

    virtual QString fileName() const override;
    virtual QString fileDisplayName() const override;
    virtual QString baseName() const override;
    virtual QString completeBaseName() const override;
    virtual bool exists() const override;
    virtual void refresh() override;
    bool canRename() const override;
    virtual QFile::Permissions permissions() const override;
    virtual bool isReadable() const override;
    virtual bool isWritable() const override;
    bool isDir() const override;
    bool canDrop() const override;

private:
    TrashFileInfoPrivate *d;
};

using TrashFileInfoPointer = QSharedPointer<TrashFileInfo>;
DPTRASH_END_NAMESPACE

#endif   // TRASHFILEINFO_H
