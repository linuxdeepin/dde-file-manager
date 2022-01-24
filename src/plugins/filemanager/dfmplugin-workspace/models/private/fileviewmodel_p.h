/*
 * Copyright (C) 2021 ~ 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huanyu<huanyub@uniontech.com>
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

#include "models/fileviewmodel.h"
#include "dfm-base/utils/fileutils.h"
#include "dfm-base/utils/traversaldirthread.h"

#include <QReadWriteLock>
#include <QQueue>
#include <QTimer>

DPWORKSPACE_BEGIN_NAMESPACE

class FileViewModelPrivate : public QObject
{
    enum EventType {
        AddFile,
        RmFile
    };
    Q_OBJECT
    friend class FileViewModel;
    FileViewModel *const q;
    dfmbase::DThreadList<FileViewItem *> childrens;
    dfmbase::DThreadMap<QUrl, FileViewItem *> childrenMap;
    QSharedPointer<FileViewItem> root;
    int column = 0;
    AbstractFileWatcherPointer watcher;
    QSharedPointer<DFMBASE_NAMESPACE::TraversalDirThread> traversalThread;
    bool canFetchMoreFlag = true;
    dfmbase::DThreadList<QUrl> handlingFileList;
    QQueue<QPair<QUrl, EventType>> watcherEvent;
    QMutex watcherEventMutex;
    QAtomicInteger<bool> isUpdatedChildren = false;
    QAtomicInteger<bool> processFileEventRuning = false;
    //文件的刷新队列
    dfmbase::DThreadList<QUrl> updateurlList;
    QTimer updateTimer;

public:
    explicit FileViewModelPrivate(FileViewModel *qq);
    virtual ~FileViewModelPrivate();
private Q_SLOTS:

    void doFileDeleted(const QUrl &url);

    void dofileAttributeChanged(const QUrl &url, const int &isExternalSource = 1)
    {
        Q_UNUSED(url);
        Q_UNUSED(isExternalSource);
    }

    void dofileMoved(const QUrl &fromUrl, const QUrl &toUrl)
    {
        doFileDeleted(fromUrl);
        dofileCreated(toUrl);
    }

    void dofileCreated(const QUrl &url);
    void doFileUpdated(const QUrl &url);
    void doFilesUpdated();
    void dofileClosed(const QUrl &url) { Q_UNUSED(url); }
    void doUpdateChildren(const QList<QUrl> &childrens);
    void doWatcherEvent();

private:
    bool checkFileEventQueue();
    QString roleDisplayString(int role);
};

DPWORKSPACE_END_NAMESPACE

#endif   // FILEVIEWMODEL_P_H
