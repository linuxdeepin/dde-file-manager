// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SEARCHDIRITERATOR_P_H
#define SEARCHDIRITERATOR_P_H

#include "searchdiriterator.h"
#include "searchmanager/searcher/abstractsearcher.h"
#include "searchmanager/searcher/searchresult_define.h"

#include <dfm-base/interfaces/fileinfo.h>
#include <dfm-base/file/local/localfilewatcher.h>

#include <QObject>
#include <QUrl>
#include <QMutex>
#include <QScopedPointer>
#include <mutex>

DFMBASE_USE_NAMESPACE

namespace dfmplugin_search {

class SearchDirIterator;
class SearchDirIteratorPrivate : public QObject
{
    Q_OBJECT
public:
    explicit SearchDirIteratorPrivate(const QUrl &url, SearchDirIterator *qq);
    ~SearchDirIteratorPrivate();

private:
    void initConnect();

public Q_SLOTS:
    void doSearch();
    void onMatched(const QString &id);
    void onSearchCompleted(const QString &id);
    void onSearchStoped(const QString &id);

public:
    QUrl fileUrl;                           // 搜索URL
    QUrl currentFileUrl;                    // 当前处理的URL
    QString currentFileContent;             // 当前处理的文件内容
    QString taskId;                         // 搜索任务ID
    quint64 winId;                          // 窗口ID

    bool searchFinished = false;            // 搜索是否完成
    bool searchStoped = false;              // 搜索是否停止

    DFMSearchResultMap childrens;    // 搜索结果集
    QScopedPointer<LocalFileWatcher> searchRootWatcher;  // 文件监视器
    mutable QMutex mutex;                   // 互斥锁
    std::once_flag searchOnceFlag;          // 一次性标志
    SearchDirIterator *q = nullptr;         // 指向父类的指针
    QWaitCondition resultWaitCond;
};

}

#endif   // SEARCHDIRITERATOR_P_H
