/*
 * Copyright (C) 2021 ~ 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liqiang<liqianga@uniontech.com>
 *
 * Maintainer: liqiang<liqianga@uniontech.com>
 *             wangchunlin<wangchunlin@uniontech.com>
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
#ifndef FILETREATER_P_H
#define FILETREATER_P_H

#include "filetreater.h"
#include "utils/threadcontainer.hpp"
#include "dfm-base/interfaces/abstractfilewatcher.h"
#include "dfm-base/interfaces/abstractfileinfo.h"
#include "dfm-base/utils/traversaldirthread.h"

#include <QTimer>
#include <QQueue>

DSB_D_BEGIN_NAMESPACE

class FileTreaterPrivate : public QObject
{
    Q_OBJECT
public:
    enum EventType {
        kAddFile,
        kRmFile,
        kReFile
    };

    explicit FileTreaterPrivate(FileTreater *q_ptr);

    void doFileDeleted(const QUrl &url);
    void doFileCreated(const QUrl &url);
    void doFileRename(const QUrl &oldUrl, const QUrl &newUrl);
    void doFileUpdated(const QUrl &url);
    void doUpdateChildren(const QList<QUrl> &childrens);
    Q_INVOKABLE void doWatcherEvent();

    inline void beginRefresh();
    inline void endRefresh();

private:
    bool checkFileEventQueue();

public:
    FileTreater *const q;

    QList<QUrl> fileList;
    QMap<QUrl, DFMLocalFileInfoPointer> fileMap;

    QSharedPointer<dfmbase::TraversalDirThread> traversalThread;
    AbstractFileWatcherPointer watcher;
    QMutex watcherEventMutex;
    QQueue<QVariant> watcherEvent;
    QDir::Filters filters = QDir::NoFilter;

    QAtomicInteger<bool> refreshedFlag = false;
    QAtomicInteger<bool> canRefreshFlag = true;
    QAtomicInteger<bool> processFileEventRuning = false;

    QUrl rootUrl;
    bool enableSort = true;
    dfmbase::AbstractFileInfo::SortKey sortRole = dfmbase::AbstractFileInfo::kSortByFileName;
    Qt::SortOrder sortOrder = Qt::AscendingOrder;

    QAtomicInteger<bool> whetherShowHiddenFile = false;
};

DSB_D_END_NAMESPACE

Q_DECLARE_METATYPE(dfm_service_desktop::FileTreaterPrivate::EventType);

#endif   // FILETREATER_P_H
