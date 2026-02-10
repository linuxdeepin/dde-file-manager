// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
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
    if (!watcher) {
        fmWarning() << "Failed to create watcher for URL:" << url.toString();
        return;
    }

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
    // 首先检查文件是否在搜索目录范围内
    auto targetUrl = SearchHelper::searchTargetUrl(this->url());
    if (!url.path().startsWith(targetUrl.path())) {
        fmDebug() << "File add ignored: not within search target path" << targetUrl.path();
        return;
    }

    auto searchKey = SearchHelper::instance()->searchKeyword(this->url());
    if (url.fileName().contains(searchKey) &&
            !dpfHookSequence->run("dfmplugin_search", "hook_Url_IsNotSubFile",
                                  targetUrl, url)) {
        fmDebug() << "File add matches search criteria:" << url.toString();
        onFileAdd(url);
    }
}

void SearchFileWatcher::handleFileDelete(const QUrl &url)
{
    // 首先检查文件是否在搜索目录范围内
    auto targetUrl = SearchHelper::searchTargetUrl(this->url());
    if (!url.path().startsWith(targetUrl.path())) {
        fmDebug() << "File delete ignored: not within search target path" << targetUrl.path();
        return;
    }

    auto searchKey = SearchHelper::instance()->searchKeyword(this->url());
    if (url.fileName().contains(searchKey) &&
            !dpfHookSequence->run("dfmplugin_search", "hook_Url_IsNotSubFile",
                                  targetUrl, url)) {
        fmDebug() << "File delete matches search criteria:" << url.toString();
        onFileDeleted(url);
    }
}

void SearchFileWatcher::handleFileRename(const QUrl &oldUrl, const QUrl &newUrl)
{
    // 首先检查文件是否在搜索目录范围内
    auto targetUrl = SearchHelper::searchTargetUrl(this->url());
    bool oldInScope = oldUrl.path().startsWith(targetUrl.path());
    bool newInScope = newUrl.path().startsWith(targetUrl.path());

    // 如果两个URL都不在搜索范围内，则直接返回
    if (!oldInScope && !newInScope) {
        fmDebug() << "File rename ignored: neither URL is within search target path";
        return;
    }

    auto searchKey = SearchHelper::instance()->searchKeyword(this->url());
    auto oldMatch = oldUrl.fileName().contains(searchKey), newMatch = newUrl.fileName().contains(searchKey);
    if (!oldMatch && !newMatch) {
        fmDebug() << "File rename ignored: neither filename matches search keyword";
        return;
    }

    auto old = dpfHookSequence->run("dfmplugin_search", "hook_Url_IsSubFile",
                                    targetUrl, oldUrl);
    auto target = dpfHookSequence->run("dfmplugin_search", "hook_Url_IsSubFile",
                                       targetUrl, newUrl);
    if ((oldMatch && old && oldInScope) && (newMatch && target && newInScope)) {
        fmDebug() << "File rename event emitted - both URLs valid:" << oldUrl.toString() << "to" << newUrl.toString();
        emit fileRename(oldUrl, newUrl);
    }

    if (old && oldMatch && oldInScope) {
        fmDebug() << "Emitting file delete for old URL:" << oldUrl.toString();
        return onFileDeleted(oldUrl);
    }

    if (target && newMatch && newInScope) {
        fmDebug() << "Emitting file add for new URL:" << newUrl.toString();
        return onFileAdd(newUrl);
    }
}
}
