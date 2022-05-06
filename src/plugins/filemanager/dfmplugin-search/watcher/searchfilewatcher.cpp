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
#include "searchfilewatcher.h"
#include "searchfilewatcher_p.h"
#include "utils/searchhelper.h"

#include "dfm-base/base/schemefactory.h"

#include <algorithm>

DPSEARCH_BEGIN_NAMESPACE

SearchFileWatcherPrivate::SearchFileWatcherPrivate(const QUrl &fileUrl, SearchFileWatcher *qq)
    : AbstractFileWatcherPrivate(fileUrl, qq)
{
}

bool SearchFileWatcherPrivate::start()
{
    bool ok = std::all_of(urlToWatcherHash.begin(), urlToWatcherHash.end(),
                          [](const AbstractFileWatcherPointer watcher) {
                              return watcher->startWatcher();
                          });

    return started = ok;
}

bool SearchFileWatcherPrivate::stop()
{
    bool ok = std::all_of(urlToWatcherHash.begin(), urlToWatcherHash.end(),
                          [](const AbstractFileWatcherPointer watcher) {
                              return watcher->stopWatcher();
                          });

    return started = ok;
}

SearchFileWatcher::SearchFileWatcher(const QUrl &url, QObject *parent)
    : AbstractFileWatcher(new SearchFileWatcherPrivate(url, this), parent)
{
    dptr = static_cast<SearchFileWatcherPrivate *>(d.data());
}

SearchFileWatcher::~SearchFileWatcher()
{
    dptr->urlToWatcherHash.clear();
}

void SearchFileWatcher::setEnabledSubfileWatcher(const QUrl &subfileUrl, bool enabled)
{
    if (enabled) {
        addWatcher(subfileUrl);
    } else {
        removeWatcher(subfileUrl);
    }
}

void SearchFileWatcher::addWatcher(const QUrl &url)
{
    if (!url.isValid() || dptr->urlToWatcherHash.contains(url))
        return;

    AbstractFileWatcherPointer watcher = WatcherFactory::create<AbstractFileWatcher>(url);
    if (!watcher)
        return;

    watcher->moveToThread(this->thread());
    connect(watcher.data(), &AbstractFileWatcher::fileAttributeChanged, this, &SearchFileWatcher::onFileAttributeChanged);
    connect(watcher.data(), &AbstractFileWatcher::fileDeleted, this, &SearchFileWatcher::onFileDeleted);
    connect(watcher.data(), &AbstractFileWatcher::fileRename, this, &SearchFileWatcher::onFileRenamed);

    dptr->urlToWatcherHash[url] = watcher;
    if (dptr->started)
        watcher->startWatcher();
}

void SearchFileWatcher::removeWatcher(const QUrl &url)
{
    dptr->urlToWatcherHash.remove(url);
}

void SearchFileWatcher::onFileDeleted(const QUrl &url)
{
    removeWatcher(url);
    emit fileDeleted(url);
}

void SearchFileWatcher::onFileAttributeChanged(const QUrl &url)
{
    emit fileAttributeChanged(url);
}

void SearchFileWatcher::onFileRenamed(const QUrl &fromUrl, const QUrl &toUrl)
{
    auto targetUrl = SearchHelper::searchTargetUrl(url());
    if (toUrl.path().startsWith(targetUrl.path())) {
        auto keyword = SearchHelper::searchKeyword(url());
        QRegularExpression regexp(keyword, QRegularExpression::CaseInsensitiveOption);
        const auto &info = InfoFactory::create<AbstractFileInfo>(toUrl);
        auto match = regexp.match(info->fileName());

        if (match.hasMatch()) {
            addWatcher(toUrl);
        }
    }

    emit fileRename(fromUrl, toUrl);
}

DPSEARCH_END_NAMESPACE
