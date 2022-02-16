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

#include "dfm-base/base/schemefactory.h"
#include "services/filemanager/search/searchservice.h"

#include <dfm-framework/framework.h>

#include <QUuid>

DPSEARCH_BEGIN_NAMESPACE

namespace GlobalPrivate {
static SearchService *searchService { nullptr };
}   // namespace GlobalPrivate

SearchDirIteratorPrivate::SearchDirIteratorPrivate(const QUrl &url, QObject *parent)
    : QObject(parent),
      fileUrl(url)
{
    loadSearchService();
    doSearch();
}

SearchDirIteratorPrivate::~SearchDirIteratorPrivate()
{
}

void SearchDirIteratorPrivate::loadSearchService()
{
    if (!GlobalPrivate::searchService) {
        auto &ctx = dpfInstance.serviceContext();
        GlobalPrivate::searchService = ctx.service<SearchService>(SearchService::name());

        if (!GlobalPrivate::searchService) {
            qCritical() << "get SearchService failed!";
            abort();
        }
    }
    initConnect();
}

void SearchDirIteratorPrivate::initConnect()
{
    connect(GlobalPrivate::searchService, &SearchService::matched, this, &SearchDirIteratorPrivate::onMatched);
    connect(GlobalPrivate::searchService, &SearchService::searchCompleted, this, &SearchDirIteratorPrivate::onSearchCompleted);
}

void SearchDirIteratorPrivate::doSearch()
{
    auto searchUrl = SearchHelper::searchTargetUrl(fileUrl);
    auto regInfos = GlobalPrivate::searchService->regInfos();
    if (UrlRoute::isVirtual(searchUrl) && regInfos.contains(searchUrl.scheme())) {
        auto path = searchUrl.path();
        auto regPath = regInfos[searchUrl.scheme()];
        if (regPath.endsWith('/') && !path.isEmpty())
            regPath = regPath.left(regPath.length() - 1);
        searchUrl = QUrl::fromLocalFile(regPath + path);
    }

    taskId = SearchHelper::searchTaskId(fileUrl);
    GlobalPrivate::searchService->search(taskId, searchUrl, SearchHelper::searchKeyword(fileUrl));
}

void SearchDirIteratorPrivate::onMatched(QString id)
{
    if (taskId == id) {
        auto results = GlobalPrivate::searchService->matchedResults(taskId);
        for (const auto &result : results) {
            QUrl url = SearchHelper::setSearchedFileUrl(fileUrl, result.toString());
            QMutexLocker lk(&mutex);
            childrens << url;
        }
    }
}

void SearchDirIteratorPrivate::onSearchCompleted(QString id)
{
    if (taskId == id) {
        qInfo() << "taskId: " << taskId << "search completed!";
        searchFinished = true;
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
        const auto &url = d->childrens.dequeue();
        lk.unlock();
        d->currentFileInfo = InfoFactory::create<AbstractFileInfo>(url);
        return url;
    }

    return {};
}

bool SearchDirIterator::hasNext() const
{
    QMutexLocker lk(&d->mutex);
    return !(d->childrens.isEmpty() && d->searchFinished);
}

QString SearchDirIterator::fileName() const
{
    return d->currentFileInfo->fileName();
}

QUrl SearchDirIterator::fileUrl() const
{
    return d->currentFileInfo->url();
}

const AbstractFileInfoPointer SearchDirIterator::fileInfo() const
{
    return d->currentFileInfo;
}

QUrl SearchDirIterator::url() const
{
    return SearchHelper::rootUrl();
}

DPSEARCH_END_NAMESPACE
