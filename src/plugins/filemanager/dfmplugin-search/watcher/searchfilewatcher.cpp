// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "searchfilewatcher.h"
#include "searchfilewatcher_p.h"
#include "utils/searchhelper.h"
#include "searchmanager/searchmanager.h"

#include <dfm-base/base/schemefactory.h>

#include <dfm-framework/event/event.h>

#include <algorithm>

namespace dfmplugin_search {

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

    started = !ok;
    return ok;
}

SearchFileWatcher::SearchFileWatcher(const QUrl &url, QObject *parent)
    : AbstractFileWatcher(new SearchFileWatcherPrivate(url, this), parent)
{
    dptr = static_cast<SearchFileWatcherPrivate *>(d.data());
    connect(SearchManager::instance(), &SearchManager::fileAdd, this,
            &SearchFileWatcher::handleFileAdd, Qt::QueuedConnection);
    connect(SearchManager::instance(), &SearchManager::fileDelete, this,
            &SearchFileWatcher::handleFileDelete, Qt::QueuedConnection);
    connect(SearchManager::instance(), &SearchManager::fileRename, this,
            &SearchFileWatcher::handleFileRename, Qt::QueuedConnection);
}

SearchFileWatcher::~SearchFileWatcher()
{
    dptr->urlToWatcherHash.clear();
}

void SearchFileWatcher::setEnabledSubfileWatcher(const QUrl &subfileUrl, bool enabled)
{
    // When 'subfileUrl' is a directory, unable to receive it rename event,
    // so monitor it's parent
    QUrl tmpUrl = subfileUrl;
    auto fileInfo = InfoFactory::create<FileInfo>(tmpUrl);
    if (fileInfo && fileInfo->fileType() == FileInfo::FileType::kDirectory)
        tmpUrl = fileInfo->urlOf(FileInfo::FileUrlInfoType::kParentUrl);

    if (enabled) {
        addWatcher(tmpUrl);
    } else {
        removeWatcher(tmpUrl);
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
    emit fileDeleted(url);
}

void SearchFileWatcher::onFileAttributeChanged(const QUrl &url)
{
    emit fileAttributeChanged(url);
}

void SearchFileWatcher::onFileRenamed(const QUrl &fromUrl, const QUrl &toUrl)
{
    bool isMatched = false;
    auto targetUrl = SearchHelper::searchTargetUrl(url());
    if (toUrl.path().startsWith(targetUrl.path())) {
        const auto &keyword = SearchHelper::instance()->checkWildcardAndToRegularExpression((SearchHelper::searchKeyword(url())));
        QRegularExpression regexp(keyword, QRegularExpression::CaseInsensitiveOption);
        const auto &info = InfoFactory::create<FileInfo>(toUrl);
        auto match = regexp.match(info->displayOf(DisPlayInfoType::kFileDisplayName));

        if (match.hasMatch()) {
            isMatched = true;
            addWatcher(toUrl);
        }
    }

    emit fileRename(fromUrl, isMatched ? toUrl : QUrl());
}

void SearchFileWatcher::onFileAdd(const QUrl &url)
{
    emit subfileCreated(url);
}

void SearchFileWatcher::handleFileAdd(const QUrl &url)
{
    auto searchKey = SearchHelper::instance()->searchKeyword(this->url());
    if (url.fileName().contains(searchKey) &&
            !dpfHookSequence->run("dfmplugin_search", "hook_Url_IsNotSubFile",
                                  SearchHelper::searchTargetUrl(this->url()), url))
        onFileAdd(url);
}

void SearchFileWatcher::handleFileDelete(const QUrl &url)
{
    auto searchKey = SearchHelper::instance()->searchKeyword(this->url());
    if (url.fileName().contains(searchKey)&&
            !dpfHookSequence->run("dfmplugin_search", "hook_Url_IsNotSubFile",
                                  SearchHelper::searchTargetUrl(this->url()), url))
        onFileDeleted(url);
}

void SearchFileWatcher::handleFileRename(const QUrl &oldUrl, const QUrl &newUrl)
{
    auto searchKey = SearchHelper::instance()->searchKeyword(this->url());
    auto oldMatch = oldUrl.fileName().contains(searchKey), newMatch = newUrl.fileName().contains(searchKey);
    if (!oldMatch && !newMatch)
        return;
    auto old = dpfHookSequence->run("dfmplugin_search", "hook_Url_IsSubFile",
                                    SearchHelper::searchTargetUrl(this->url()), oldUrl);
    auto target = dpfHookSequence->run("dfmplugin_search", "hook_Url_IsSubFile",
                                       SearchHelper::searchTargetUrl(this->url()), newUrl);
    if ((oldMatch && old) && (newMatch &&target)) {
        emit fileRename(oldUrl, newUrl);
    }

    if (old && oldMatch)
        return onFileDeleted(oldUrl);

    if (target && newMatch)
        return onFileAdd(newUrl);
}

}
