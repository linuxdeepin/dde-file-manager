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

#include "dfm-base/base/schemefactory.h"
#include "services/filemanager/search/searchservice.h"

#include <dfm-framework/framework.h>

#include <QUuid>

DSB_FM_USE_NAMESPACE
DPSEARCH_BEGIN_NAMESPACE

SearchDirIteratorPrivate::SearchDirIteratorPrivate(const QUrl &url, QObject *parent)
    : QObject(parent),
      fileUrl(url)
{
    initConnect();
    doSearch();
}

SearchDirIteratorPrivate::~SearchDirIteratorPrivate()
{
}

void SearchDirIteratorPrivate::initConnect()
{
    connect(SearchService::service(), &SearchService::matched, this, &SearchDirIteratorPrivate::onMatched);
    connect(SearchService::service(), &SearchService::searchCompleted, this, &SearchDirIteratorPrivate::onSearchCompleted);
    connect(SearchService::service(), &SearchService::searchStoped, this, &SearchDirIteratorPrivate::onSearchStoped);
}

void SearchDirIteratorPrivate::doSearch()
{
    auto searchUrl = SearchHelper::searchTargetUrl(fileUrl);
    auto regInfos = SearchService::service()->regInfos();
    if (UrlRoute::isVirtual(searchUrl) && regInfos.contains(searchUrl.scheme())) {
        auto path = searchUrl.path();
        auto regPath = regInfos[searchUrl.scheme()];
        if (regPath.endsWith('/') && !path.isEmpty())
            regPath = regPath.left(regPath.length() - 1);
        searchUrl = QUrl::fromLocalFile(regPath + path);
    }

    taskId = SearchHelper::searchTaskId(fileUrl);
    SearchService::service()->search(taskId, searchUrl, SearchHelper::searchKeyword(fileUrl));
}

void SearchDirIteratorPrivate::onMatched(const QString &id)
{
    if (taskId == id) {
        auto results = SearchService::service()->matchedResults(taskId);
        for (const auto &result : results) {
            QUrl url = SearchHelper::setSearchedFileUrl(fileUrl, result.toString());
            QMutexLocker lk(&mutex);
            childrens << url;
        }
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
    if (taskId == id)
        searchStoped = true;
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
        d->currentFileUrl = d->childrens.dequeue();
        return d->currentFileUrl;
    }

    return {};
}

bool SearchDirIterator::hasNext() const
{
    if (d->searchStoped) {
        SearchEventCaller::sendStopSpinner(d->taskId.toULongLong());
        return false;
    }

    QMutexLocker lk(&d->mutex);
    bool hasNext = !(d->childrens.isEmpty() && d->searchFinished);
    if (!hasNext)
        SearchEventCaller::sendStopSpinner(d->taskId.toULongLong());
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

DPSEARCH_END_NAMESPACE
