// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
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
                SearchManager::instance()->stop(taskId);
                SearchEventCaller::sendChangeCurrentUrl(winId, QUrl("computer:///"));
            }
        });
    }

    bool isDisable = CustomManager::instance()->isDisableSearch(targetUrl);
    if (isDisable) return;

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
        // 从SearchManager获取结果，这些结果已经经过合并处理
        const auto &results = SearchManager::instance()->matchedResults(taskId);
        if (results.isEmpty())
            return;
            
        QMutexLocker lk(&mutex);
        childrens = results;
    }

    resultWaitCond.wakeAll();
}

void SearchDirIteratorPrivate::onSearchCompleted(const QString &id)
{
    if (taskId == id) {
        fmInfo() << "taskId: " << taskId << "search completed!";
        searchFinished = true;
    }

    resultWaitCond.wakeAll();
}

void SearchDirIteratorPrivate::onSearchStoped(const QString &id)
{
    if (taskId == id) {
        searchStoped = true;
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
        d->searchStoped = false;
        emit this->sigSearch();
    });
    
    // Lock and wait for children to be populated or search to stop/finish.
    QMutexLocker lk(&d->mutex);
    while (d->childrens.isEmpty() && !d->searchStoped) {
        if (d->searchFinished)
            break;
        d->resultWaitCond.wait(&d->mutex);
    }
    if (d->searchFinished && d->childrens.isEmpty())
        return {};

    for (auto it = d->childrens.begin(); it != d->childrens.end(); ++it) {
        auto sortInfo = QSharedPointer<SortFileInfo>(new SortFileInfo());
        sortInfo->setUrl(it.key());
        sortInfo->setHighlightContent(it->highlightedContent());
        doCompleteSortInfo(sortInfo);
        result.append(sortInfo);
    }
    d->childrens.clear();
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
    // 2. Search is not finished 
    // 3. Search is not stopped
    
    // Since we can't directly check if once_flag has been called,
    // we can infer it from the taskId not being empty
    
    // Protect access to shared variables
    QMutexLocker lk(&d->mutex);
    
    return !d->taskId.isEmpty() && !d->searchFinished && !d->searchStoped;
}

void SearchDirIterator::doCompleteSortInfo(SortInfoPointer sortInfo)
{
    if (!sortInfo || sortInfo->isInfoCompleted())
        return;

    QUrl url = sortInfo->fileUrl();

    if (!url.isLocalFile())
        return;

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

             // 设置 MIME 类型显示名称（这个不需要额外的文件系统调用）
    sortInfo->setDisplayType(MimeTypeDisplayManager::instance()->displayTypeFromPath(url.path()));

             // 标记所有信息已完成
    sortInfo->setInfoCompleted(true);
}

}
