// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "searchdiriterator.h"
#include "searchdiriterator_p.h"
#include "utils/searchhelper.h"
#include "utils/custommanager.h"
#include "events/searcheventcaller.h"
#include "searchmanager/searchmanager.h"

#include <dfm-base/base/schemefactory.h>
#include <dfm-base/utils/universalutils.h>
#include <dfm-base/file/local/localfilewatcher.h>
#include <dfm-base/utils/fileutils.h>
#include <dfm-base/mimetype/mimetypedisplaymanager.h>

#include <QUuid>

#include <sys/stat.h>
#include <algorithm>

DFMBASE_USE_NAMESPACE
DPSEARCH_USE_NAMESPACE

namespace dfmplugin_search {

SearchDirIteratorPrivate::SearchDirIteratorPrivate(const QUrl &url, SearchDirIterator *qq)
    : QObject(qq),
      fileUrl(url),
      q(qq)
{
    initConnect();
}

SearchDirIteratorPrivate::~SearchDirIteratorPrivate()
{
}

void SearchDirIteratorPrivate::initConnect()
{
    connect(q, &SearchDirIterator::sigSearch, this, &SearchDirIteratorPrivate::doSearch);
    connect(q, &SearchDirIterator::sigStopSearch, this, [this]() {
        SearchEventCaller::sendStopSpinner(winId);
    });

    connect(SearchManager::instance(), &SearchManager::matched, this, &SearchDirIteratorPrivate::onMatched);
    connect(SearchManager::instance(), &SearchManager::searchCompleted, this, &SearchDirIteratorPrivate::onSearchCompleted);
    connect(SearchManager::instance(), &SearchManager::searchStoped, this, &SearchDirIteratorPrivate::onSearchStoped);
}

void SearchDirIteratorPrivate::doSearch()
{
    auto targetUrl = SearchHelper::searchTargetUrl(fileUrl);
    if (targetUrl.isLocalFile()) {
        searchRootWatcher.reset(new LocalFileWatcher(targetUrl));
        searchRootWatcher->startWatcher();
        connect(searchRootWatcher.data(), &LocalFileWatcher::fileDeleted, this, [=](const QUrl &url) {
            if (UniversalUtils::urlEquals(targetUrl, url)) {
                fmWarning() << "Search target deleted, stopping search:" << url;
                SearchManager::instance()->stop(taskId);
                SearchEventCaller::sendChangeCurrentUrl(winId, QUrl("computer:///"));
            }
        });
    }

    bool isDisable = CustomManager::instance()->isDisableSearch(targetUrl);
    if (isDisable) {
        fmDebug() << "Search disabled for target:" << targetUrl;
        return;
    }

    QString redirectedPath = CustomManager::instance()->redirectedPath(targetUrl);
    if (!redirectedPath.isEmpty()) {
        targetUrl = QUrl::fromLocalFile(redirectedPath);
    }

    winId = SearchHelper::searchWinId(fileUrl).toULongLong();
    taskId = QUuid::createUuid().toString(QUuid::WithoutBraces);
    SearchEventCaller::sendStartSpinner(winId);
    SearchManager::instance()->search(winId, taskId, targetUrl, SearchHelper::searchKeyword(fileUrl));
}

void SearchDirIteratorPrivate::onMatched(const QString &id)
{
    if (taskId == id) {
        // 直接在主线程更新结果，但使用高效的双缓冲机制
        const auto &results = SearchManager::instance()->matchedResults(taskId);
        if (!results.isEmpty()) {
            resultBuffer.updateResults(results);
            hasConsumedResults.store(false, std::memory_order_release);   // 标记有新数据
        }

        // 通知等待的消费者
        QMutexLocker lk(&waitMutex);
        resultWaitCond.wakeAll();
    }
}

void SearchDirIteratorPrivate::onSearchCompleted(const QString &id)
{
    if (taskId == id) {
        fmInfo() << "taskId: " << taskId << "search completed!";
        searchFinished.store(true, std::memory_order_release);
    }

    resultWaitCond.wakeAll();
}

void SearchDirIteratorPrivate::onSearchStoped(const QString &id)
{
    if (taskId == id) {
        searchStoped.store(true, std::memory_order_release);
        emit q->sigStopSearch();
        if (searchRootWatcher)
            searchRootWatcher->stopWatcher();
    }

    resultWaitCond.wakeAll();
}

SearchDirIterator::SearchDirIterator(const QUrl &url,
                                     const QStringList &nameFilters,
                                     QDir::Filters filters,
                                     QDirIterator::IteratorFlags flags)
    : AbstractDirIterator(url, nameFilters, filters, flags),
      d(new SearchDirIteratorPrivate(url, this))
{
    setProperty(IteratorProperty::kKeepOrder, true);
}

SearchDirIterator::~SearchDirIterator()
{
}

QUrl SearchDirIterator::next()
{
    return {};
}

bool SearchDirIterator::hasNext() const
{
    return false;
}

QString SearchDirIterator::fileName() const
{
    return "";
}

QUrl SearchDirIterator::fileUrl() const
{
    return d->currentFileUrl;
}

const FileInfoPointer SearchDirIterator::fileInfo() const
{
    return nullptr;
}

QUrl SearchDirIterator::url() const
{
    return SearchHelper::rootUrl();
}

QList<QSharedPointer<SortFileInfo>> SearchDirIterator::sortFileInfoList()
{
    QList<QSharedPointer<SortFileInfo>> result;

    // 确保搜索已经开始
    std::call_once(d->searchOnceFlag, [this]() {
        d->searchStoped.store(false, std::memory_order_release);
        emit this->sigSearch();
    });

    // 等待搜索结果或搜索完成/停止
    {
        QMutexLocker lk(&d->waitMutex);
        while ((d->resultBuffer.isEmpty() || d->hasConsumedResults.load(std::memory_order_acquire)) && !d->searchStoped.load(std::memory_order_acquire)) {
            if (d->searchFinished.load(std::memory_order_acquire))
                break;
            d->resultWaitCond.wait(&d->waitMutex);
        }
    }

    // 修复：搜索完成时，仍需检查是否有未消费的结果
    // 只有在搜索完成且缓冲区为空且已消费过结果的情况下才返回空
    if (d->searchFinished.load(std::memory_order_acquire) && d->resultBuffer.isEmpty() && d->hasConsumedResults.load(std::memory_order_acquire))
        return {};

    const auto results = d->resultBuffer.consumeResults();

    // 如果没有新结果且搜索已完成，返回空
    if (results.isEmpty() && d->searchFinished.load(std::memory_order_acquire))
        return {};

    // TODO (perf) : 存在性能问题，重复获取全量数据
    // 在子线程中处理数据，不影响主线程
    // 使用两个QList分别装载文件夹和文件，然后合并
    // 时间复杂度：O(n)，空间复杂度：O(n)，性能优于stable_partition
    QList<QSharedPointer<SortFileInfo>> dirs;
    QList<QSharedPointer<SortFileInfo>> files;

    // 预分配空间以提高性能（可选优化）
    const int totalCount = results.size();
    dirs.reserve(totalCount / 4);   // 估算文件夹占比约25%
    files.reserve(totalCount);   // 文件可能占大部分

    for (auto it = results.begin(); it != results.end(); ++it) {
        auto sortInfo = QSharedPointer<SortFileInfo>(new SortFileInfo());
        sortInfo->setUrl(it.key());
        sortInfo->setHighlightContent(it->highlightedContent());
        doCompleteSortInfo(sortInfo);

        if (sortInfo->isDir()) {
            dirs.append(sortInfo);
        } else {
            files.append(sortInfo);
        }
    }

    // 合并结果：文件夹在前，文件在后
    result = std::move(dirs);
    result.append(files);

    // 标记结果已被消费，避免重复处理
    d->hasConsumedResults.store(true, std::memory_order_release);

    return result;
}

void SearchDirIterator::close()
{
    if (d->taskId.isEmpty())
        return;

    SearchManager::instance()->stop(d->taskId);
}

bool SearchDirIterator::isWaitingForUpdates() const
{
    // We're waiting for updates if:
    // 1. Search has started (check if the once_flag has been called)
    // 2. Search is not finished OR the result buffer is not empty
    // 3. Search is not stopped

    // Since we can't directly check if once_flag has been called,
    // we can infer it from the taskId not being empty.

    // We are waiting for updates if the search is not finished yet,
    // OR if the search is finished but there are still results in the buffer.
    const bool hasPendingResults = !d->resultBuffer.isEmpty();
    const bool isSearchInProgress = !d->searchFinished.load(std::memory_order_acquire);
    return !d->taskId.isEmpty()
            && (isSearchInProgress || hasPendingResults)
            && !d->searchStoped.load(std::memory_order_acquire);
}

void SearchDirIterator::doCompleteSortInfo(SortInfoPointer sortInfo)
{
    if (!sortInfo || sortInfo->isInfoCompleted())
        return;

    QUrl url = sortInfo->fileUrl();

    if (!url.isLocalFile()) {
        fmWarning() << "Cannot complete sort info for non-local file:" << url;
        return;
    }

    struct stat64 statBuffer;
    const QString filePath = url.path();

    if (::stat64(filePath.toUtf8().constData(), &statBuffer) != 0)
        return;

    // 一次性设置所有从 stat64 获取的信息

    // 基础信息
    sortInfo->setSize(statBuffer.st_size);
    sortInfo->setFile(S_ISREG(statBuffer.st_mode));
    sortInfo->setDir(S_ISDIR(statBuffer.st_mode));
    sortInfo->setSymlink(S_ISLNK(statBuffer.st_mode));

    // 隐藏文件检查
    QString fileName = url.fileName();
    sortInfo->setHide(fileName.startsWith('.'));

    // 权限信息
    sortInfo->setReadable(statBuffer.st_mode & S_IRUSR);
    sortInfo->setWriteable(statBuffer.st_mode & S_IWUSR);
    sortInfo->setExecutable(statBuffer.st_mode & S_IXUSR);

    // 时间信息
    sortInfo->setLastReadTime(statBuffer.st_atime);
    sortInfo->setLastModifiedTime(statBuffer.st_mtime);
    sortInfo->setCreateTime(statBuffer.st_ctime);

    // 标记所有信息已完成
    sortInfo->setInfoCompleted(true);
}

// ======== SearchResultBuffer 实现 ========

void SearchResultBuffer::updateResults(const DFMSearchResultMap &newResults)
{
    QMutexLocker lock(&writerMutex);

    // 写入非活跃缓冲区
    if (useBufferA.load(std::memory_order_acquire)) {
        bufferB = newResults;
        useBufferA.store(false, std::memory_order_release);   // 原子切换
    } else {
        bufferA = newResults;
        useBufferA.store(true, std::memory_order_release);   // 原子切换
    }
}

DFMSearchResultMap SearchResultBuffer::getResults() const
{
    // 无锁读取活跃缓冲区
    return useBufferA.load(std::memory_order_acquire) ? bufferA : bufferB;
}

DFMSearchResultMap SearchResultBuffer::consumeResults()
{
    QMutexLocker lock(&writerMutex);
    DFMSearchResultMap results;
    if (useBufferA.load(std::memory_order_acquire)) {
        results.swap(bufferA);
    } else {
        results.swap(bufferB);
    }
    return results;
}

bool SearchResultBuffer::isEmpty() const
{
    // 检查当前活跃缓冲区是否为空
    return useBufferA.load(std::memory_order_acquire) ? bufferA.isEmpty() : bufferB.isEmpty();
}

}
