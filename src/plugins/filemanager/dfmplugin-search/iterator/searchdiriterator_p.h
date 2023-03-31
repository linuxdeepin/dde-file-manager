// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SEARCHDIRITERATOR_P_H
#define SEARCHDIRITERATOR_P_H

#include "dfmplugin_search_global.h"

#include <dfm-base/interfaces/fileinfo.h>

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
    quint64 winId;
    QString taskId;
    QMutex mutex;
    std::once_flag onceFlag;
    std::once_flag searchOnceFlag;

    SearchDirIterator *q;
    QScopedPointer<DFMBASE_NAMESPACE::LocalFileWatcher> searchRootWatcher;
};

}

#endif   // SEARCHDIRITERATOR_P_H
