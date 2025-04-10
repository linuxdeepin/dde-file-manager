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

#include <QUuid>

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
}

void SearchDirIteratorPrivate::onSearchCompleted(const QString &id)
{
    if (taskId == id) {
        fmInfo() << "taskId: " << taskId << "search completed!";
        searchFinished = true;
    }
}

void SearchDirIteratorPrivate::onSearchStoped(const QString &id)
{
    if (taskId == id) {
        searchStoped = true;
        emit q->sigStopSearch();
        if (searchRootWatcher)
            searchRootWatcher->stopWatcher();
    }
}

SearchDirIterator::SearchDirIterator(const QUrl &url,
                                     const QStringList &nameFilters,
                                     QDir::Filters filters,
                                     QDirIterator::IteratorFlags flags)
    : AbstractDirIterator(url, nameFilters, filters, flags),
      d(new SearchDirIteratorPrivate(url, this))
{
}

SearchDirIterator::~SearchDirIterator()
{
}

QUrl SearchDirIterator::next()
{
    if (!d->childrens.isEmpty()) {
        QMutexLocker lk(&d->mutex);
        // 从结果映射中取第一个结果的URL
        auto it = d->childrens.begin();
        d->currentFileUrl = it.key();
        d->currentFileContent = it->highlightedContent();
        d->childrens.erase(it);
        return d->currentFileUrl;
    }

    return {};
}

bool SearchDirIterator::hasNext() const
{
    std::call_once(d->searchOnceFlag, [this]() {
        d->searchStoped = false;
        emit this->sigSearch();
    });

    if (d->searchStoped) {
        emit sigStopSearch();
        return false;
    }

    QMutexLocker lk(&d->mutex);
    bool hasNext = !(d->childrens.isEmpty() && d->searchFinished);
    if (!hasNext)
        emit sigStopSearch();
    return hasNext;
}

QString SearchDirIterator::fileName() const
{
    return fileInfo()->nameOf(NameInfoType::kFileName);
}

QUrl SearchDirIterator::fileUrl() const
{
    return d->currentFileUrl;
}

const FileInfoPointer SearchDirIterator::fileInfo() const
{
    if (!d->currentFileUrl.isValid())
        return nullptr;

    auto info = InfoFactory::create<FileInfo>(d->currentFileUrl);

    if (!d->currentFileContent.isEmpty()) {
        // 设置高亮内容到扩展属性
        info->setExtendedAttributes(ExtInfoType::kFileHighlightContent, d->currentFileContent);
        d->currentFileContent.clear();
    }

    return info;
}

QUrl SearchDirIterator::url() const
{
    return SearchHelper::rootUrl();
}

void SearchDirIterator::close()
{
    if (d->taskId.isEmpty())
        return;

    SearchManager::instance()->stop(d->taskId);
}

}
