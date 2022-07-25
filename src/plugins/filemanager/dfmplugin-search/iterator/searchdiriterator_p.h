/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liuzhangjian<liuzhangjian@uniontech.com>
 *
 * Maintainer: liuzhangjian<liuzhangjian@uniontech.com>
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
#ifndef SEARCHDIRITERATOR_P_H
#define SEARCHDIRITERATOR_P_H

#include "dfmplugin_search_global.h"

#include "dfm-base/interfaces/abstractfileinfo.h"

#include <QObject>
#include <QQueue>
#include <QUrl>
#include <QMutex>

#include <mutex>

namespace dfmbase {
class LocalFileWatcher;
}

namespace dfmplugin_search {

class SearchDirIterator;
class SearchDirIteratorPrivate : public QObject
{
    Q_OBJECT
    friend class SearchDirIterator;

public:
    explicit SearchDirIteratorPrivate(const QUrl &url, SearchDirIterator *qq);
    ~SearchDirIteratorPrivate();

    void initConnect();

public slots:
    void doSearch();
    void onMatched(const QString &id);
    void onSearchCompleted(const QString &id);
    void onSearchStoped(const QString &id);

private:
    bool searchFinished = false;
    bool searchStoped = false;
    QUrl fileUrl;
    QList<QUrl> childrens;
    QUrl currentFileUrl;
    QString taskId;
    QMutex mutex;
    std::once_flag onceFlag;
    std::once_flag searchOnceFlag;

    SearchDirIterator *q;
    QScopedPointer<DFMBASE_NAMESPACE::LocalFileWatcher> searchRootWatcher;
};

}

#endif   // SEARCHDIRITERATOR_P_H
