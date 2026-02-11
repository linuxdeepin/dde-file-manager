// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "iteratorsearcher.h"
#include "utils/searchhelper.h"

#include <dfm-base/utils/fileutils.h>
#include <dfm-base/base/schemefactory.h>

#include <QDebug>
#include <QDirIterator>
#include <QFileInfo>
#include <QApplication>
#include <QMetaObject>
#include <QTimer>

DFMBASE_USE_NAMESPACE
DPSEARCH_USE_NAMESPACE

// ================= IteratorSearcherBridge 实现 =================
IteratorSearcherBridge::IteratorSearcherBridge(QObject *parent)
    : QObject(parent)
{
    // 确保桥接类在主线程创建
    Q_ASSERT(thread() == qApp->thread());
}

IteratorSearcherBridge::~IteratorSearcherBridge() = default;

void IteratorSearcherBridge::setSearcher(IteratorSearcher *searcher)
{
    // 弱引用指向搜索器
    searcherPtr = searcher;

    connect(searcherPtr, &IteratorSearcher::requestCreateIterator,
            this, &IteratorSearcherBridge::createIterator,
            Qt::QueuedConnection);
    connect(this, &IteratorSearcherBridge::iteratorCreated,
            searcherPtr, &IteratorSearcher::onIteratorCreated,
            Qt::QueuedConnection);
}

void IteratorSearcherBridge::createIterator(const QUrl &url)
{
    // 始终在主线程中执行
    Q_ASSERT(thread() == qApp->thread());

    // 创建目录迭代器
    auto iterator = DirIteratorFactory::create(url, QStringList(),
                                            QDir::NoDotAndDotDot | QDir::Dirs | QDir::Files);

    if (!iterator) {
        fmWarning() << "Failed to create directory iterator for URL:" << url.toString();
        // 如果创建失败，发出空迭代器信号
        emit iteratorCreated(nullptr);
        return;
    }

    // 设置查询属性
    iterator->setProperty("QueryAttributes", "standard::name,standard::type,standard::size,"
                        "standard::is-symlink,standard::symlink-target,access::*,time::*");

    // 发送给工作线程
    emit iteratorCreated(iterator);
}

// ================= IteratorSearcher 实现 =================
IteratorSearcher::IteratorSearcher(const QUrl &url, const QString &key, QObject *parent)
    : AbstractSearcher(url, SearchHelper::instance()->checkWildcardAndToRegularExpression(key), parent),
      status(kReady),
      batchTimer(new QTimer(this)),
      batchResultLimit(200),       // 默认批处理大小限制为200个结果
      batchTimeLimit(500)          // 默认批处理时间限制为500毫秒
{
    // 创建正则表达式，忽略大小写
    regex = QRegularExpression(keyword, QRegularExpression::CaseInsensitiveOption);

    // 连接处理目录的信号
    connect(this, &IteratorSearcher::requestProcessNextDirectory,
            this, &IteratorSearcher::processDirectory,
            Qt::QueuedConnection);

    // 配置批处理定时器
    batchTimer->setSingleShot(true);
    connect(batchTimer, &QTimer::timeout, this, &IteratorSearcher::publishBatchedResults);
}

IteratorSearcher::~IteratorSearcher()
{
    // 清理资源
    pendingDirs.clear();

    // 确保停止定时器
    if (batchTimer->isActive())
        batchTimer->stop();
}

bool IteratorSearcher::search()
{
    // 原子状态转换：Ready -> Running
    if (!status.testAndSetRelease(kReady, kRuning)) {
        fmWarning() << "Failed to start search - invalid state transition, current status:" << status.loadAcquire();
        return false;
    }

    // 从根URL开始搜索
    pendingDirs.enqueue(searchUrl);

    // 创建桥接类实例，总是在主线程中运行
    // 确保桥接类在主线程上创建
    QObject *mainThread = qApp;
    QMetaObject::invokeMethod(mainThread, [this]() {
        // 这个lambda在主线程中运行
        bridge = QSharedPointer<IteratorSearcherBridge>::create();
        bridge->setSearcher(this);

        // 通知构造函数bridge已准备好
        emit requestProcessNextDirectory();
    }, Qt::BlockingQueuedConnection);

    return true;
}

void IteratorSearcher::stop()
{
    // 标记为终止状态
    QAtomicInt previousState = status.fetchAndStoreRelease(kTerminated);

    // 只在之前为运行状态时执行清理
    if (previousState == kRuning) {
        // 清理待处理目录
        pendingDirs.clear();

        // 确保处理挖掘的结果
        if (hasItem())
            emit unearthed(this);

        // 通知搜索完成
        emit finished();
    }
}

bool IteratorSearcher::hasItem() const
{
    QMutexLocker lk(&mutex);
    return !resultMap.isEmpty();
}

DFMSearchResultMap IteratorSearcher::takeAll()
{
    QMutexLocker lk(&mutex);
    DFMSearchResultMap results = resultMap;
    resultMap.clear();
    return results;
}

QList<QUrl> IteratorSearcher::takeAllUrls()
{
    QMutexLocker lk(&mutex);
    QList<QUrl> urls;
    urls.reserve(resultMap.size());

    for (auto it = resultMap.constBegin(); it != resultMap.constEnd(); ++it) {
        urls << it.key();
    }

    resultMap.clear();
    return urls;
}

void IteratorSearcher::processDirectory()
{
    // 检查状态
    if (status.loadAcquire() != kRuning) {
        fmDebug() << "Directory processing skipped - not in running state";
        return;
    }

    // 如果队列为空且状态为运行中，标记为完成
    if (pendingDirs.isEmpty()) {
        status.storeRelease(kCompleted);
        fmDebug() << "Iterator search completed - no more directories to process";
        emit finished();
        return;
    }

    // 从队列获取下一个目录
    QUrl currentUrl = pendingDirs.dequeue();

    // 在主线程中请求创建迭代器
    emit requestCreateIterator(currentUrl);
}

void IteratorSearcher::onIteratorCreated(QSharedPointer<DFMBASE_NAMESPACE::AbstractDirIterator> iterator)
{
    // 检查状态
    if (status.loadAcquire() != kRuning) {
        fmDebug() << "Iterator creation callback ignored - not in running state";
        return;
    }

    // 处理迭代器结果
    if (iterator) {
        processIteratorResults(iterator);
    } else {
        fmWarning() << "Received null iterator from bridge";
    }

    // 请求处理下一个目录
    requestNextDirectory();
}

void IteratorSearcher::processIteratorResults(QSharedPointer<DFMBASE_NAMESPACE::AbstractDirIterator> iterator)
{
    if (status.loadAcquire() != kRuning)
        return;

    DFMSearchResultMap newResults;
    QList<QUrl> subDirs;

    // 使用迭代器处理目录条目
    while (iterator->hasNext() && status.loadAcquire() == kRuning) {
        QUrl fileUrl = iterator->next();
        auto info = iterator->fileInfo();

        if (!info || !info->exists())
            continue;

        // 添加子目录到搜索队列
        if (info->isAttributes(OptInfoType::kIsDir) && !info->isAttributes(OptInfoType::kIsSymLink)) {
            const auto &dirUrl = info->urlOf(UrlInfoType::kUrl);
            if (!dirUrl.path().startsWith("/sys/")) {
                subDirs << dirUrl;
            }
        }

        // The keyword check (the regex)
        if (regex.match(info->displayOf(DisPlayInfoType::kFileDisplayName)).hasMatch())
            addResultToMap(fileUrl, newResults);
    }

    // 将子目录添加到队列
    for (const QUrl &subDir : subDirs) {
        pendingDirs.enqueue(subDir);
    }

    // 处理结果
    if (!newResults.isEmpty() && status.loadAcquire() == kRuning)
        addResults(newResults);
}

void IteratorSearcher::addResultToMap(const QUrl &fileUrl, DFMSearchResultMap &results)
{
    // 创建搜索结果
    DFMSearchResult result;
    result.setUrl(fileUrl);
    result.setMatchScore(1.0);  // 默认匹配分数

    // 添加到结果
    results.insert(fileUrl, result);
}

void IteratorSearcher::addResults(const DFMSearchResultMap &newResults)
{
    if (newResults.isEmpty())
        return;

    int currentBatchSize = 0;
    // 批量处理结果
    {
        QMutexLocker lk(&mutex);
        for (auto it = newResults.constBegin(); it != newResults.constEnd(); ++it) {
            // 同时添加到总结果集
            resultMap.insert(it.key(), it.value());

            // 将新结果合并到批处理结果中
            batchedResults.insert(it.key(), it.value());
        }
        currentBatchSize = batchedResults.size();
    }

    // 当积累的批处理结果达到阈值时，或首次有结果时，立即发布
    if (currentBatchSize >= batchResultLimit || !batchTimer->isActive())
        publishBatchedResults();
}

void IteratorSearcher::publishBatchedResults()
{
    // 检查状态
    if (status.loadAcquire() != kRuning)
        return;

    // 只有当有结果时才通知
    if (!batchedResults.isEmpty()) {
        {
            QMutexLocker lk(&mutex);
            // 清空批处理结果，下一批重新开始
            batchedResults.clear();
        }

        // 通知新结果
        emit unearthed(this);
    }

    // 重新计时
    batchTimer->start(batchTimeLimit);
}

void IteratorSearcher::requestNextDirectory()
{
    // 通过信号请求处理下一个目录
    emit requestProcessNextDirectory();
}
