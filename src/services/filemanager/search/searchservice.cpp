/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
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
#include "searchservice.h"
#include "private/searchservice_p.h"
#include "fulltextsearch/fulltextsearch.h"

#include <QtConcurrent>

SearchServicePrivate::SearchServicePrivate()
    : createIndexSearcher(new FullTextSearch)
{
}

SearchServicePrivate::~SearchServicePrivate()
{
}

SearchService::SearchService(QObject *parent)
    : dpf::PluginService(parent),
      dpf::AutoServiceRegister<SearchService>(),
      d(new SearchServicePrivate)
{
}

SearchService::~SearchService()
{
    if (!d->fullTextSearchers.isEmpty()) {
        for (const auto &searcher : d->fullTextSearchers.values())
            searcher->stop();
        d->fullTextSearchers.clear();
    }

    if (d->createIndexSearcher && d->createIndexFuture.isRunning()) {
        d->createIndexSearcher->stop();
        d->createIndexFuture.waitForFinished();
    }
}

QStringList SearchService::fulltextSearch(quint64 winId, const QString &path, const QString &keyword)
{
    if (!d->fullTextSearchers.contains(winId)) {
        QSharedPointer<FullTextSearch> searcher(new FullTextSearch);
        d->fullTextSearchers.insert(winId, searcher);
    }

    return d->fullTextSearchers[winId]->search(path, keyword);
}

bool SearchService::createFullTextIndex(const QString &path)
{
    if (d->createIndexFuture.isRunning())
        return false;

    d->createIndexFuture = QtConcurrent::run([=]() {
        d->createIndexSearcher->createIndex(path);
    });

    return true;
}

void SearchService::updateFullTextIndex(quint64 winId, const QString &path)
{
    if (!d->fullTextSearchers.contains(winId))
        return;

    d->fullTextSearchers[winId]->updateIndex(path);
}

bool SearchService::hasUpdated(quint64 winId)
{
    if (!d->fullTextSearchers.contains(winId))
        return false;

    return d->fullTextSearchers[winId]->isUpdated();
}

QStringList SearchService::fileSearch(quint64 winId, const QString &path, const QString &keyword, bool useRegExp)
{
    // TODO(liuzhangjian)
    return {};
}

QStringList SearchService::fileSearch(quint64 winId, qint32 maxCount, qint64 maxTime, quint32 &startOffset, quint32 &endOffset,
                                      const QString &path, const QString &keyword, bool useRegExp)
{
    // TODO(liuzhangjian)
    return {};
}

void SearchService::stop(quint64 winId)
{
    if (d->fullTextSearchers.contains(winId)) {
        d->fullTextSearchers[winId]->stop();
        d->fullTextSearchers.remove(winId);
    }
}
