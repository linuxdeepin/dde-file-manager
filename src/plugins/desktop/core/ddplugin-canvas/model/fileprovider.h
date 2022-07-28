/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangyu<zhangyub@uniontech.com>
 *
 * Maintainer: zhangyu<zhangyub@uniontech.com>
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
#ifndef FILEPROVIDER_H
#define FILEPROVIDER_H

#include "ddplugin_canvas_global.h"

#include <dfm-base/utils/traversaldirthread.h>
#include <interfaces/abstractfilewatcher.h>

#include <QObject>
#include <QMutex>
#include <QUrl>

namespace ddplugin_canvas {
class FileFilter;
class FileProvider : public QObject
{
    Q_OBJECT
public:
    explicit FileProvider(QObject *parent = nullptr);
    bool setRoot(const QUrl &url);
    QUrl root() const;
    bool isUpdating() const;
    void refresh(QDir::Filters filters = QDir::NoFilter);
    void installFileFilter(QSharedPointer<FileFilter> filter);
    void removeFileFilter(QSharedPointer<FileFilter> filter);
signals:
    void refreshEnd(const QList<QUrl> &urls);
    void fileRemoved(const QUrl &url);
    void fileInserted(const QUrl &url);
    void fileRenamed(const QUrl &oldurl, const QUrl &newurl);
    void fileUpdated(const QUrl &url);
protected slots:
    void traversalFinished();
    void reset(QList<QUrl> children);
    void insert(const QUrl &url);
    void remove(const QUrl &url);
    void rename(const QUrl &oldUrl, const QUrl &newUrl);
    void update(const QUrl &url);
protected:
    QUrl rootUrl;
    AbstractFileWatcherPointer watcher;
    QList<QSharedPointer<FileFilter>> fileFilters;
private:
    QAtomicInteger<bool> updateing = false;
    QSharedPointer<DFMBASE_NAMESPACE::TraversalDirThread> traversalThread;
};

}

#endif // FILEPROVIDER_H
