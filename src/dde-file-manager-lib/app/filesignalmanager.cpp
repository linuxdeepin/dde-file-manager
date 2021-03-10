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

#include "filesignalmanager.h"

#include "dfmevent.h"
#include "dabstractfileinfo.h"
#include "durl.h"
#include <QDir>


FileSignalManager::FileSignalManager(QObject *parent):
    QObject(parent)
{
    qRegisterMetaType<DFMEvent>(QT_STRINGIFY(DFMEvent));
    qRegisterMetaType<DFMUrlListBaseEvent>(QT_STRINGIFY(DFMUrlListBaseEvent));
    qRegisterMetaType<QDir::Filters>(QT_STRINGIFY(QDir::Filters));
    qRegisterMetaType<QList<DAbstractFileInfoPointer>>(QT_STRINGIFY(QList<DAbstractFileInfoPointer>));
    qRegisterMetaType<DUrl>(QT_STRINGIFY(DUrl));
    qRegisterMetaType<DUrl>();
    qRegisterMetaType<DUrlList>();
}
