/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huangyu<huangyub@uniontech.com>
 *
 * Maintainer: huangyu<huangyub@uniontech.com>
 *             zhangyu<zhangyub@uniontech.com>
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
#ifndef ABSTRACTCANVASMODEL_H
#define ABSTRACTCANVASMODEL_H

#include "dfm-base/dfm_base_global.h"

#include <QAbstractItemModel>

DFMBASE_BEGIN_NAMESPACE
class AbstractCanvasModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    // todo: 待剔除非桌面使用部分
    enum Roles {
        FileIconRole = Qt::DecorationRole,
        FilePathRole = Qt::UserRole + 1,
        FileNameRole = Qt::UserRole + 2,
        FileSizeRole = Qt::UserRole + 3,
        FileMimeTypeRole = Qt::UserRole + 4,
        FileOwnerRole = Qt::UserRole + 5,
        FileLastModifiedRole = Qt::UserRole + 6,
        FileLastReadRole = Qt::UserRole + 7,
        FileCreatedRole = Qt::UserRole + 8,
        FileDisplayNameRole = Qt::UserRole + 9,
        FilePinyinName = Qt::UserRole + 10,
        ExtraProperties = Qt::UserRole + 11,
        FileBaseNameRole = Qt::UserRole + 12,
        FileSuffixRole = Qt::UserRole + 13,
        FileNameOfRenameRole = Qt::UserRole + 14,
        FileBaseNameOfRenameRole = Qt::UserRole + 15,
        FileSuffixOfRenameRole = Qt::UserRole + 16,
        FileSizeInKiloByteRole = Qt::UserRole + 17,
        FileLastModifiedDateTimeRole = Qt::UserRole + 18,
        FileIconModelToolTipRole = Qt::UserRole + 19,
        FileLastReadDateTimeRole = Qt::UserRole + 20,
        FileCreatedDateTimeRole = Qt::UserRole + 21,
        FileUserRole = Qt::UserRole + 99,
        UnknowRole = Qt::UserRole + 999
    };

    explicit AbstractCanvasModel(QObject *parent = nullptr);
};
DFMBASE_END_NAMESPACE

#endif   // ABSTRACTCANVASMODEL_H
