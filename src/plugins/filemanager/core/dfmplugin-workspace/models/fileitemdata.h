/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liuyangming<liuyangming@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             max-lv<lvwujun@uniontech.com>
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
#ifndef FILEITEMDATA_H
#define FILEITEMDATA_H

#include "dfmplugin_workspace_global.h"

#include "dfm-base/interfaces/abstractfileinfo.h"
#include "dfm-base/dfm_base_global.h"

#include <QSharedData>

namespace dfmplugin_workspace {

class FileItemData : public QObject
{
public:
    explicit FileItemData(const QUrl &url, FileItemData *parent = nullptr);

    void setParentData(FileItemData *p);

    void refreshInfo();
    AbstractFileInfoPointer fileInfo() const;
    FileItemData *parentData() const;

    QVariant data(int role) const;

private:
    FileItemData *parent;
    AbstractFileInfoPointer info;
};

}

#endif   // FILEITEMDATA_H
