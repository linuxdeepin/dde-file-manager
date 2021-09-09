/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huanyu<huanyu@uniontech.com>
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
#include "widgets/dfmfileview/fileviewmodel.h"
#include "widgets/dfmfileview/private/traversaldirthread.h"
#include "dfm-base/utils/threadcontainer.hpp"

#include <QReadWriteLock>
#include <QQueue>

DFMBASE_BEGIN_NAMESPACE
class FileViewModelPrivate : public QObject
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(FileViewModel)
    FileViewModel * q_ptr;
    DThreadList<QSharedPointer<FileViewItem>> childers;
    QSharedPointer<FileViewItem> root;
    int column = 0;
    AbstractFileWatcherPointer watcher;
    QSharedPointer<TraversalDirThread> traversalThread;
public:
    enum EventType {
        AddFile,
        RmFile
    };
    explicit FileViewModelPrivate(FileViewModel *qq);
    virtual ~FileViewModelPrivate();

private Q_SLOTS:
    void doFileDeleted(const QUrl &url){Q_UNUSED(url);}
    void dofileAttributeChanged(const QUrl &url, const int &isExternalSource = 1){Q_UNUSED(url);Q_UNUSED(isExternalSource);}
    void dofileMoved(const QUrl &fromUrl, const QUrl &toUrl){Q_UNUSED(fromUrl);Q_UNUSED(toUrl);}
    void dofileCreated(const QUrl &url){Q_UNUSED(url);}
    void dofileModified(const QUrl &url){Q_UNUSED(url);}
    void dofileClosed(const QUrl &url){Q_UNUSED(url);}
    void doUpdateChildren(const QList<QSharedPointer<FileViewItem>> &children);
};
DFMBASE_END_NAMESPACE

#endif // FILEVIEWMODEL_P_H
