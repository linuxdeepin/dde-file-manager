// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "taskcommander.h"
#include "taskcommander_p.h"
#include "searchmanager/searcher/abstractsearcher.h"
#include "searchmanager/searcher/dfmsearch/dfmsearcher.h"
#include "searchmanager/searcher/iterator/iteratorsearcher.h"
#include "utils/searchhelper.h"

#include <dfm-base/base/urlroute.h>
#include <dfm-base/base/configs/dconfig/dconfigmanager.h>

#include <dfm-search/dsearch_global.h>

#include <QDebug>

DFMBASE_USE_NAMESPACE
DPSEARCH_USE_NAMESPACE
DFM_SEARCH_USE_NS

// ======== SimplifiedSearchWorker 实现 ========

SimplifiedSearchWorker::SimplifiedSearchWorker(QObject *parent)
    : QObject(parent),
      isRunning(false),
      finishedSearcherCount(0)
{
}

SimplifiedSearchWorker::~SimplifiedSearchWorker()
{
    stopSearch();
}

DFMSearchResultMap SimplifiedSearchWorker::getResults()
{
    QReadLocker locker(&rwLock);
    return resultMap;
}

QList<QUrl> SimplifiedSearchWorker::getResultUrls()
{
    QReadLocker locker(&rwLock);
    return resultMap.keys();
}

void SimplifiedSearchWorker::startSearch()
{
    // 重置状态
    isRunning = true;
    finishedSearcherCount = 0;

    {
        QWriteLocker locker(&rwLock);
        resultMap.clear();
    }

    // 创建搜索器并启动搜索
    createSearchers();
}

void SimplifiedSearchWorker::stopSearch()
{
    isRunning = false;

    cleanupSearchers();
}

void SimplifiedSearchWorker::createSearchers()
{
    if (DFMSearcher::supportUrl(searchUrl)) {
        const QString &searchPath = DFMSearcher::realSearchPath(searchUrl);
        const QStringList &indexedPaths = DFMSEARCH::Global::defaultIndexedDirectory();

        // 检查搜索路径与索引路径的关系
        bool isParentOfAnyIndexedPath = false;
        QStringList relevantIndexedPaths;

        for (const QString &indexedPath : indexedPaths) {
            if (isParentPath(searchPath, indexedPath)) {
                isParentOfAnyIndexedPath = true;
                relevantIndexedPaths.append(indexedPath);
            }
        }

        // 当searchPath是任何indexedPath的父路径
        if (isParentOfAnyIndexedPath) {
            // 创建当前searchUrl的搜索器
            createSearchersForUrl(searchUrl);

            // 为每个相关的索引路径创建搜索器
            for (const QString &indexedPath : relevantIndexedPaths) {
                QUrl indexedUrl = QUrl::fromLocalFile(indexedPath);
                createSearchersForUrl(indexedUrl);
            }
        } else {
            // 当searchPath不是任何indexedPath的父路径时，只搜索当前路径
            createSearchersForUrl(searchUrl);
        }

        return;
    }

    // 使用IteratorSearcher作为文件系统搜索器
    IteratorSearcher *searcher = new IteratorSearcher(searchUrl, searchKeyword, this);

    // 连接信号以接收搜索结果和完成通知
    connect(searcher, &AbstractSearcher::unearthed, this, &SimplifiedSearchWorker::onSearcherUnearthed);
    connect(searcher, &AbstractSearcher::finished, this, &SimplifiedSearchWorker::onSearcherFinished);

    searchers.append(searcher);

    // 启动搜索
    searcher->search();
}

bool SimplifiedSearchWorker::isParentPath(const QString &parentPath, const QString &childPath) const
{
    // 确保路径格式一致（带尾部斜杠）
    QString normalizedParent = parentPath.endsWith('/') ? parentPath : parentPath + '/';

    // 检查childPath是否以parentPath开头
    return childPath.startsWith(normalizedParent);
}

void SimplifiedSearchWorker::createSearchersForUrl(const QUrl &url)
{
    // 支持的搜索类型
    QList<SearchType> searchTypes = { SearchType::FileName };

    // 检查是否开启了全文搜索
    bool enableContentSearch = DConfigManager::instance()->value(
                                                                 DConfig::kSearchCfgPath, DConfig::kEnableFullTextSearch, false)
                                       .toBool();

    if (enableContentSearch) {
        searchTypes.append(SearchType::Content);
    }

    // 为每种搜索类型创建搜索器
    for (auto type : searchTypes) {
        // 使用DFMSearcher作为默认搜索器
        AbstractSearcher *searcher = new DFMSearcher(url, searchKeyword, this, type);

        // 连接信号
        connect(searcher, &AbstractSearcher::unearthed, this, &SimplifiedSearchWorker::onSearcherUnearthed);
        connect(searcher, &AbstractSearcher::finished, this, &SimplifiedSearchWorker::onSearcherFinished);

        searchers.append(searcher);

        // 启动搜索
        searcher->search();
    }
}

void SimplifiedSearchWorker::cleanupSearchers()
{
    // 停止并删除所有搜索器
    for (auto searcher : searchers) {
        searcher->disconnect(this);
        searcher->stop();

        // 延迟删除搜索器以确保主线程回调完成
        searcher->deleteLater();
    }

    searchers.clear();
}

void SimplifiedSearchWorker::onSearcherUnearthed()
{
    // 这个槽会在搜索器找到结果时被调用
    AbstractSearcher *searcher = qobject_cast<AbstractSearcher *>(sender());
    if (searcher && isRunning) {
        // 合并结果
        mergeResults(searcher);

        // 通知UI更新结果
        emit resultsUpdated(taskId);
    }
}

void SimplifiedSearchWorker::mergeResults(AbstractSearcher *searcher)
{
    if (!searcher || !searcher->hasItem())
        return;

    // 获取新结果
    DFMSearchResultMap newResults = searcher->takeAll();

    if (newResults.isEmpty())
        return;

    // 合并到主结果集
    {
        QWriteLocker locker(&rwLock);

        // 第一次直接复制结果
        if (resultMap.isEmpty()) {
            resultMap = newResults;
            return;
        }

        // 合并结果，保留匹配分数更高的结果
        for (auto it = newResults.constBegin(); it != newResults.constEnd(); ++it) {
            const QUrl &url = it.key();
            auto existing = resultMap.find(url);

            if (existing != resultMap.end()) {
                if (it.value().matchScore() > existing.value().matchScore())
                    resultMap[url] = it.value();
            } else {
                resultMap.insert(url, it.value());
            }
        }
    }
}

void SimplifiedSearchWorker::onSearcherFinished()
{
    AbstractSearcher *searcher = qobject_cast<AbstractSearcher *>(sender());
    if (!searcher)
        return;

    if (!searchers.contains(searcher))
        return;

    // 最后一次检查是否有新结果
    if (searcher->hasItem() && isRunning) {
        mergeResults(searcher);
        emit resultsUpdated(taskId);
    }

    // 移除完成的搜索器
    searchers.removeAll(searcher);
    searcher->deleteLater();

    // 所有搜索器完成时通知搜索完成
    if (searchers.isEmpty() && isRunning) {
        // 通知搜索完成
        emit searchCompleted(taskId);

        // 标记搜索结束
        isRunning = false;
    }
}

// ======== TaskCommanderPrivate 实现 ========

TaskCommanderPrivate::TaskCommanderPrivate(TaskCommander *parent)
    : QObject(parent),
      q(parent),
      deleted(false)
{
    // 创建搜索工作线程
    searchWorker = new SimplifiedSearchWorker;
    searchWorker->moveToThread(&workerThread);

    // 连接信号
    connect(&workerThread, &QThread::finished, searchWorker, &QObject::deleteLater);
    connect(searchWorker, &SimplifiedSearchWorker::resultsUpdated,
            this, &TaskCommanderPrivate::onResultsUpdated);
    connect(searchWorker, &SimplifiedSearchWorker::searchCompleted,
            this, &TaskCommanderPrivate::onSearchCompleted);
    connect(&workerThread, &QThread::finished,
            q, &TaskCommander::onWorkThreadFinished);

    // 启动工作线程
    workerThread.start();
}

TaskCommanderPrivate::~TaskCommanderPrivate()
{
}

void TaskCommanderPrivate::onResultsUpdated(const QString &id)
{
    if (id == taskId && !deleted) {
        emit q->matched(taskId);
    }
}

void TaskCommanderPrivate::onSearchCompleted(const QString &id)
{
    if (id == taskId && !deleted) {
        emit q->finished(taskId);
    }
}

// ======== TaskCommander 实现 ========

TaskCommander::TaskCommander(QString taskId, const QUrl &url, const QString &keyword, QObject *parent)
    : QObject(parent)
{
    d = new TaskCommanderPrivate(this);
    d->taskId = taskId;

    // 初始化并设置搜索参数
    if (d->searchWorker) {
        QMetaObject::invokeMethod(
                d->searchWorker, [=]() {
                    d->searchWorker->setTaskId(taskId);
                    d->searchWorker->setSearchUrl(url);
                    d->searchWorker->setKeyword(keyword);
                },
                Qt::QueuedConnection);
    } else {
        fmWarning() << "Failed to create search worker for task:" << taskId;
    }
}

TaskCommander::~TaskCommander()
{
}

QString TaskCommander::taskID() const
{
    return d->taskId;
}

DFMSearchResultMap TaskCommander::getResults() const
{
    if (!d->searchWorker) {
        fmWarning() << "Search worker not available for getting results";
        return DFMSearchResultMap();
    }

    DFMSearchResultMap results;
    QMetaObject::invokeMethod(d->searchWorker, "getResults", Qt::DirectConnection,
                              Q_RETURN_ARG(DFMSearchResultMap, results));

    return results;
}

QList<QUrl> TaskCommander::getResultsUrls() const
{
    if (!d->searchWorker) {
        fmWarning() << "Search worker not available for getting result URLs";
        return QList<QUrl>();
    }

    QList<QUrl> results;
    QMetaObject::invokeMethod(d->searchWorker, "getResultUrls", Qt::DirectConnection,
                              Q_RETURN_ARG(QList<QUrl>, results));

    return results;
}

bool TaskCommander::start()
{
    if (!d->searchWorker) {
        fmWarning() << "Cannot start search, search worker not available";
        return false;
    }

    QMetaObject::invokeMethod(d->searchWorker, "startSearch", Qt::QueuedConnection);
    return true;
}

void TaskCommander::stop()
{
    if (!d->searchWorker)
        deleteLater();

    QMetaObject::invokeMethod(d->searchWorker, "stopSearch", Qt::QueuedConnection);
    d->workerThread.quit();
}

void TaskCommander::onWorkThreadFinished()
{
    d->deleted = true;
}
