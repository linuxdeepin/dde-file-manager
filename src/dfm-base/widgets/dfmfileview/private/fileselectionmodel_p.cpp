/*
 * Copyright (C) 2021 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liuyangming<liuyangming@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
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
#include "fileselectionmodel_p.h"

#include <QItemSelectionModel>

DFMBASE_BEGIN_NAMESPACE

FileSelectionModelPrivate::FileSelectionModelPrivate(FileSelectionModel *qq)
    : QObject(qq),
      q(qq)
{
    timer.setSingleShot(true);
    QObject::connect(&timer, &QTimer::timeout, this, &FileSelectionModelPrivate::updateSelecteds);
}

void FileSelectionModelPrivate::updateSelecteds()
{
    q->select(selection, currentCommand);
}

DFMBASE_END_NAMESPACE
