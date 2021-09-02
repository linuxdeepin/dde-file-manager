/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     yanghao<yanghao@uniontech.com>
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
#ifndef FILEVIEWMODEL_P_H
#define FILEVIEWMODEL_P_H

#include "shutil/fileutils.h"
#include "shutil/dfmfilelistfile.h"
#include "widgets/dfmfileview/fileviewmodel.h"
#include "widgets/dfmfileview/private/traversaldirthread.h"
#include "dfm-base/utils/threadcontainer.hpp"

#include <QReadWriteLock>
#include <QQueue>

class FileViewModelPrivate
{
    FileViewModel * q_ptr;

public:
    enum EventType {
        AddFile,
        RmFile
    };
    explicit FileViewModelPrivate(FileViewModel *qq);
    ~FileViewModelPrivate();

private:
    DThreadList<QSharedPointer<DFMFileViewItem>> childers;
    QSharedPointer<DFMFileViewItem> root;
    int column = 0;

    AbstractFileWatcherPointer watcher;
    QSharedPointer<DFMTraversalDirThread> traversalThread;

    Q_DECLARE_PUBLIC(FileViewModel)
};

#endif // FILEVIEWMODEL_P_H
