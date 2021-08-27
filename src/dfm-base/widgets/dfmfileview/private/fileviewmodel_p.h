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
#ifndef DFMFileViewModel_P_H
#define DFMFileViewModel_P_H

#include "shutil/fileutils.h"
#include "shutil/dfmfilelistfile.h"
#include "widgets/dfmfileview/fileviewmodel.h"
#include "widgets/dfmfileview/private/traversaldirthread.h"
#include "dfm-base/utils/threadcontainer.hpp"

#include <QReadWriteLock>
#include <QQueue>

class DFMFileViewModelPrivate
{
    DFMFileViewModel * q_ptr;

public:
    enum EventType {
        AddFile,
        RmFile
    };
    explicit DFMFileViewModelPrivate(DFMFileViewModel *qq);
    ~DFMFileViewModelPrivate();

private:
    DThreadList<QSharedPointer<DFMFileViewItem>> m_childers;
    QSharedPointer<DFMFileViewItem> m_root;
    int m_column = 0;

    DAbstractFileWatcherPointer m_watcher;
    QSharedPointer<DFMTraversalDirThread> m_traversalThread;

    Q_DECLARE_PUBLIC(DFMFileViewModel)
};

#endif // DFMFileViewModel_P_H
