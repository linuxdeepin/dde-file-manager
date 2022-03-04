/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     wangchunlin<wangchunlin@uniontech.com>
 *
 * Maintainer: wangchunlin<wangchunlin@uniontech.com>
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
#ifndef CANVASMODELPRIVATE_H
#define CANVASMODELPRIVATE_H

#include "canvasmodel.h"

#include <dfm-base/utils/traversaldirthread.h>
#include <interfaces/abstractfilewatcher.h>

#include <QMutex>
#include <QQueue>

DDP_CANVAS_BEGIN_NAMESPACE

class FileTreater;
class CanvasModelPrivate : public QObject
{
    Q_OBJECT
public:
    enum EventType {
        kAddFile,
        kRmFile,
        kReFile,
        kUpdateFile
    };

    explicit CanvasModelPrivate(CanvasModel *qq);
    virtual ~CanvasModelPrivate();
    Q_INVOKABLE void doWatcherEvent();

    void delayRefresh(int ms = 50);
    void doRefresh();

public slots:
    void onTraversalFinished();
    void onFileDeleted(const QUrl &url);
    void onFileCreated(const QUrl &url);
    void onFileRename(const QUrl &oldUrl, const QUrl &newUrl);
    void onFileUpdated(const QUrl &url);

private:
    bool checkFileEventQueue();

public:
    CanvasModel *const q;
    QUrl rootUrl;
    QSharedPointer<FileTreater> fileTreater;

    QSharedPointer<QTimer> refreshTimer;
    QSharedPointer<dfmbase::TraversalDirThread> traversalThread;
    AbstractFileWatcherPointer watcher;
    QMutex watcherEventMutex;
    QQueue<QVariant> watcherEvent;
    dfmio::DEnumerator::DirFilters filters = dfmio::DEnumerator::DirFilter::NoFilter;

    QAtomicInteger<bool> isUpdatedChildren = false;
    QAtomicInteger<bool> processFileEventRuning = false;
    QAtomicInteger<bool> whetherShowHiddenFile = false;
};
DDP_CANVAS_END_NAMESPACE

Q_DECLARE_METATYPE(DDP_CANVAS_NAMESPACE::CanvasModelPrivate::EventType);

#endif   // CANVASMODELPRIVATE_H
