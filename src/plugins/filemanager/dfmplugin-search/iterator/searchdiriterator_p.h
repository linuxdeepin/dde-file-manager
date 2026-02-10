// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
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
#include <QWaitCondition>
#include <atomic>
#include <mutex>

DFMBASE_USE_NAMESPACE

namespace dfmplugin_search {

// 双缓冲搜索结果管理器
class SearchResultBuffer
{
public:
    SearchResultBuffer() = default;
    ~SearchResultBuffer() = default;

    // 生产者：更新搜索结果（主线程调用）
    void updateResults(const DFMSearchResultMap &newResults);

    // 消费者：获取当前搜索结果快照（子线程调用）
    DFMSearchResultMap getResults() const;

    // 消费者：获取并清空当前搜索结果（子线程调用）
    DFMSearchResultMap consumeResults();

    // 检查是否有数据
    bool isEmpty() const;

private:
    mutable DFMSearchResultMap bufferA;
    mutable DFMSearchResultMap bufferB;
    std::atomic<bool> useBufferA { true };
    mutable QMutex writerMutex;   // 只保护写操作
};

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
    QUrl fileUrl;   // 搜索URL
    QUrl currentFileUrl;   // 当前处理的URL
    QString currentFileContent;   // 当前处理的文件内容
    QString taskId;   // 搜索任务ID
    quint64 winId;   // 窗口ID

    std::atomic<bool> searchFinished { false };   // 搜索是否完成(原子操作保证线程安全)
    std::atomic<bool> searchStoped { false };   // 搜索是否停止(原子操作保证线程安全)

    SearchResultBuffer resultBuffer;   // 双缓冲搜索结果
    QScopedPointer<LocalFileWatcher> searchRootWatcher;   // 文件监视器
    std::once_flag searchOnceFlag;   // 一次性标志
    SearchDirIterator *q = nullptr;   // 指向父类的指针
    QWaitCondition resultWaitCond;
    mutable QMutex waitMutex;   // 只用于等待条件的轻量级锁
    std::atomic<bool> hasConsumedResults { false };   // 标记结果是否已被消费(原子操作保证线程安全)
};

}

#endif   // SEARCHDIRITERATOR_P_H
