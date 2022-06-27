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
#include "searchdiriterator.h"
#include "searchdiriterator_p.h"
#include "utils/searchhelper.h"
#include "events/searcheventcaller.h"

#include "services/filemanager/search/searchservice.h"
#include "dfm-base/base/schemefactory.h"
#include "dfm-base/utils/universalutils.h"
#include "dfm-base/file/local/localfilewatcher.h"

#include <dfm-framework/framework.h>

DSB_FM_USE_NAMESPACE
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
        SearchEventCaller::sendStopSpinner(taskId.toULongLong());
    });
    connect(SearchService::service(), &SearchService::matched, this, &SearchDirIteratorPrivate::onMatched);
    connect(SearchService::service(), &SearchService::searchCompleted, this, &SearchDirIteratorPrivate::onSearchCompleted);
    connect(SearchService::service(), &SearchService::searchStoped, this, &SearchDirIteratorPrivate::onSearchStoped);
}

void SearchDirIteratorPrivate::doSearch()
{
    auto targetUrl = SearchHelper::searchTargetUrl(fileUrl);
    const auto &info = SearchService::service()->findCustomSearchInfo(targetUrl.scheme());
    DFMBASE_USE_NAMESPACE
    searchRootWatcher.reset(new LocalFileWatcher(targetUrl));
    searchRootWatcher->startWatcher();
    connect(searchRootWatcher.data(), &LocalFileWatcher::fileDeleted, this, [=](const QUrl &url) {
        if (UniversalUtils::urlEquals(targetUrl, url)) {
            SearchService::service()->stop(taskId);
            SearchEventCaller::sendChangeCurrentUrl(taskId.toULongLong(), QUrl("computer:///"));
        }
    });

    if (info.isDisableSearch) return;

    if (!info.redirectedPath.isEmpty()) {
        auto targetPath = targetUrl.path();
        auto redirectedPath = info.redirectedPath;
        if (redirectedPath.endsWith('/') && !targetPath.isEmpty())
            redirectedPath = redirectedPath.left(redirectedPath.length() - 1);
        targetUrl = QUrl::fromLocalFile(redirectedPath + targetPath);
    }

    taskId = SearchHelper::searchTaskId(fileUrl);
    SearchEventCaller::sendStartSpinner(taskId.toULongLong());
    SearchService::service()->search(taskId, targetUrl, SearchHelper::searchKeyword(fileUrl));
}

void SearchDirIteratorPrivate::onMatched(const QString &id)
{
    if (taskId == id) {
        auto results = SearchService::service()->matchedResults(taskId);
        QMutexLocker lk(&mutex);
        childrens.append(std::move(results));
        lk.unlock();

        std::call_once(onceFlag, [this]() {
            SearchEventCaller::sendShowAdvanceSearchButton(taskId.toULongLong(), true);
        });
    }
}

void SearchDirIteratorPrivate::onSearchCompleted(const QString &id)
{
    if (taskId == id) {
        qInfo() << "taskId: " << taskId << "search completed!";
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
        d->currentFileUrl = d->childrens.takeFirst();
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
    return fileInfo()->fileName();
}

QUrl SearchDirIterator::fileUrl() const
{
    return d->currentFileUrl;
}

const AbstractFileInfoPointer SearchDirIterator::fileInfo() const
{
    return InfoFactory::create<AbstractFileInfo>(d->currentFileUrl);
}

QUrl SearchDirIterator::url() const
{
    return SearchHelper::rootUrl();
}

void SearchDirIterator::close()
{
    if (d->taskId.isEmpty())
        return;

    SearchService::service()->stop(d->taskId);
}

}
