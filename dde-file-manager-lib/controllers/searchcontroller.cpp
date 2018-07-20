/*
 * Copyright (C) 2016 ~ 2018 Deepin Technology Co., Ltd.
 *               2016 ~ 2018 dragondjf
 *
 * Author:     dragondjf<dingjiangfeng@deepin.com>
 *
 * Maintainer: dragondjf<dingjiangfeng@deepin.com>
 *             zccrs<zhangjide@deepin.com>
 *             Tangtong<tangtong@deepin.com>
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

#include "searchcontroller.h"
#include "dfileservices.h"
#include "dfmevent.h"
#include "dfileproxywatcher.h"
#include "private/dabstractfilewatcher_p.h"

#include "models/searchfileinfo.h"
#include "ddiriterator.h"

#include "app/define.h"
#include "app/filesignalmanager.h"

#include <DDesktopServices>

#include <QDebug>
#include <QRegularExpression>
#include <QQueue>
#include <QRegExp>

QString searchKeywordPattern(const QString &keyword)
{
    QString keywordPattern = keyword;
    if (!keyword.contains('*') && !keyword.contains('?')) {
        keywordPattern.prepend('*');
        keywordPattern.append('*');
    }
    return keywordPattern;
}

class SearchFileWatcherPrivate;
class SearchFileWatcher : public DAbstractFileWatcher
{
public:
    explicit SearchFileWatcher(const DUrl &url, QObject *parent = 0);
    ~SearchFileWatcher();

    void setEnabledSubfileWatcher(const DUrl &subfileUrl, bool enabled = true) Q_DECL_OVERRIDE;

private:
    void addWatcher(const DUrl &url);
    void removeWatcher(const DUrl &url);

    void onFileDeleted(const DUrl &url);
    void onFileAttributeChanged(const DUrl &url);
    void onFileMoved(const DUrl &fromUrl, const DUrl &toUrl);
    void onSubfileCreated(const DUrl &url);
    void onFileModified(const DUrl &url);

    Q_DECLARE_PRIVATE(SearchFileWatcher)
};

class SearchFileWatcherPrivate : public DAbstractFileWatcherPrivate
{
public:
    SearchFileWatcherPrivate(SearchFileWatcher *qq)
        : DAbstractFileWatcherPrivate(qq) {}

    bool start() Q_DECL_OVERRIDE;
    bool stop() Q_DECL_OVERRIDE;

    QMap<DUrl, DAbstractFileWatcher *> urlToWatcherMap;

    Q_DECLARE_PUBLIC(SearchFileWatcher)
};

SearchFileWatcher::SearchFileWatcher(const DUrl &url, QObject *parent)
    : DAbstractFileWatcher(*new SearchFileWatcherPrivate(this), url, parent)
{

}

SearchFileWatcher::~SearchFileWatcher()
{
    Q_D(SearchFileWatcher);
    d->urlToWatcherMap.clear();
}

void SearchFileWatcher::setEnabledSubfileWatcher(const DUrl &subfileUrl, bool enabled)
{
    DUrl url = subfileUrl;

    url.setSearchedFileUrl(DUrl());

    if (url != fileUrl()) {
        return;
    }

    if (enabled) {
        addWatcher(subfileUrl.searchedFileUrl());
    } else {
        removeWatcher(subfileUrl.searchedFileUrl());
    }
}

void SearchFileWatcher::addWatcher(const DUrl &url)
{
    Q_D(SearchFileWatcher);

    if (!url.isValid() || d->urlToWatcherMap.contains(url)) {
        return;
    }

    DAbstractFileWatcher *watcher = DFileService::instance()->createFileWatcher(this, url);

    if (!watcher) {
        return;
    }

    watcher->moveToThread(this->thread());
    watcher->setParent(this);

    d->urlToWatcherMap[url] = watcher;

    connect(watcher, &DAbstractFileWatcher::fileAttributeChanged, this, &SearchFileWatcher::onFileAttributeChanged);
    connect(watcher, &DAbstractFileWatcher::fileDeleted, this, &SearchFileWatcher::onFileDeleted);
    connect(watcher, &DAbstractFileWatcher::fileModified, this, &SearchFileWatcher::onFileModified);
    connect(watcher, &DAbstractFileWatcher::fileMoved, this, &SearchFileWatcher::onFileMoved);

    if (d->started) {
        watcher->startWatcher();
    }
}

void SearchFileWatcher::removeWatcher(const DUrl &url)
{
    Q_D(SearchFileWatcher);

    DAbstractFileWatcher *watcher = d->urlToWatcherMap.take(url);

    if (!watcher) {
        return;
    }

    watcher->deleteLater();
}

void SearchFileWatcher::onFileDeleted(const DUrl &url)
{
    removeWatcher(url);

    DUrl newUrl = fileUrl();
    newUrl.setSearchedFileUrl(url);

    emit fileDeleted(newUrl);
}

void SearchFileWatcher::onFileAttributeChanged(const DUrl &url)
{
    DUrl newUrl = fileUrl();
    newUrl.setSearchedFileUrl(url);

    emit fileAttributeChanged(newUrl);
}

void SearchFileWatcher::onFileMoved(const DUrl &fromUrl, const DUrl &toUrl)
{
    Q_D(SearchFileWatcher);

    DUrl newFromUrl = fileUrl();
    newFromUrl.setSearchedFileUrl(fromUrl);

    DUrl newToUrl = toUrl;
    if (fileUrl().searchTargetUrl().scheme() == toUrl.scheme() && toUrl.path().startsWith(fileUrl().searchTargetUrl().path())) {
        QString keywordPattern = searchKeywordPattern(fileUrl().searchKeyword());
        const DAbstractFileInfoPointer &info = DFileService::instance()->createFileInfo(this, toUrl);
        QRegExp regular = QRegExp(keywordPattern, Qt::CaseInsensitive, QRegExp::Wildcard);
        if (regular.exactMatch(info->fileDisplayName())) {
            newToUrl = fileUrl();
            newToUrl.setSearchedFileUrl(toUrl);

            addWatcher(toUrl);
        }
    }

    removeWatcher(fromUrl);

    emit fileMoved(newFromUrl, newToUrl);
}

void SearchFileWatcher::onFileModified(const DUrl &url)
{
    DUrl newUrl = fileUrl();
    newUrl.setSearchedFileUrl(url);

    emit fileModified(newUrl);
}

bool SearchFileWatcherPrivate::start()
{
    bool ok = true;

    for (DAbstractFileWatcher *watcher : urlToWatcherMap) {
        ok = ok && watcher->startWatcher();
    }

    started = ok;

    return ok;
}

bool SearchFileWatcherPrivate::stop()
{
    bool ok = true;

    for (DAbstractFileWatcher *watcher : urlToWatcherMap) {
        ok = ok && watcher->stopWatcher();
    }

    started = !ok;

    return ok;
}

class SearchDiriterator : public DDirIterator
{
public:
    SearchDiriterator(const DUrl &url, const QStringList &nameFilters, QDir::Filters filter,
                      QDirIterator::IteratorFlags flags, SearchController *parent);
    ~SearchDiriterator();

    DUrl next() Q_DECL_OVERRIDE;
    bool hasNext() const Q_DECL_OVERRIDE;

    QString fileName() const Q_DECL_OVERRIDE;
    DUrl fileUrl() const Q_DECL_OVERRIDE;
    const DAbstractFileInfoPointer fileInfo() const Q_DECL_OVERRIDE;
    DUrl url() const Q_DECL_OVERRIDE;
    void close() Q_DECL_OVERRIDE;

    SearchController *parent;
    DAbstractFileInfoPointer currentFileInfo;
    mutable QQueue<DUrl> childrens;

    DUrl m_fileUrl;
    DUrl targetUrl;
    QString keyword;
    QRegExp regular;
    QStringList m_nameFilters;
    QDir::Filters m_filter;
    QDirIterator::IteratorFlags m_flags;
    mutable QList<DUrl> searchPathList;
    mutable DDirIteratorPointer it;
    mutable bool m_hasIteratorByKeywordOfCurrentIt;

    bool closed = false;
};

SearchDiriterator::SearchDiriterator(const DUrl &url, const QStringList &nameFilters,
                                     QDir::Filters filter, QDirIterator::IteratorFlags flags,
                                     SearchController *parent)
    : DDirIterator()
    , parent(parent)
    , m_fileUrl(url)
    , m_nameFilters(nameFilters)
    , m_filter(filter)
    , m_flags(flags)
{
    targetUrl = url.searchTargetUrl();
    keyword = searchKeywordPattern(url.searchKeyword());

    regular = QRegExp(keyword, Qt::CaseInsensitive, QRegExp::Wildcard);
    searchPathList << targetUrl;
}

SearchDiriterator::~SearchDiriterator()
{

}

DUrl SearchDiriterator::next()
{
    if (!childrens.isEmpty()) {
        const DUrl &url = childrens.dequeue();

        currentFileInfo = DFileService::instance()->createFileInfo(parent, url);

        return url;
    }

    return DUrl();
}

bool SearchDiriterator::hasNext() const
{
    if (!childrens.isEmpty()) {
        return true;
    }

    forever {
        if (closed) {
            return false;
        }

        if (!it) {
            if (searchPathList.isEmpty()) {
                break;
            }

            const DUrl &url = searchPathList.takeAt(0);

            it = DFileService::instance()->createDirIterator(parent, url, m_nameFilters, QDir::NoDotAndDotDot | m_filter, m_flags);

            if (!it) {
                continue;
            }

            m_hasIteratorByKeywordOfCurrentIt = it->enableIteratorByKeyword(m_fileUrl.searchKeyword());
        }

        while (it->hasNext()) {
            if (closed) {
                return false;
            }

            it->next();

            DAbstractFileInfoPointer fileInfo = it->fileInfo();

            fileInfo->makeAbsolute();

            if (m_hasIteratorByKeywordOfCurrentIt) {
                DUrl url = m_fileUrl;
                const DUrl &realUrl = fileInfo->fileUrl();

                url.setSearchedFileUrl(realUrl);
                childrens << url;

                return true;
            }

            if (fileInfo->isDir() && !fileInfo->isSymLink()) {
                const DUrl &url = fileInfo->fileUrl();

                if (!searchPathList.contains(url)) {
                    searchPathList << url;
                }
            }

            if (regular.exactMatch(fileInfo->fileDisplayName())) {
                DUrl url = m_fileUrl;
                const DUrl &realUrl = fileInfo->fileUrl();

                url.setSearchedFileUrl(realUrl);

                childrens << url;

                return true;
            }
        }

        it.clear();
    }

    return false;
}

QString SearchDiriterator::fileName() const
{
    return currentFileInfo ? currentFileInfo->fileName() : QString();
}

DUrl SearchDiriterator::fileUrl() const
{
    return currentFileInfo ? currentFileInfo->fileUrl() : DUrl();
}

const DAbstractFileInfoPointer SearchDiriterator::fileInfo() const
{
    return currentFileInfo;
}

DUrl SearchDiriterator::url() const
{
    return m_fileUrl;
}

void SearchDiriterator::close()
{
    closed = true;
}

SearchController::SearchController(QObject *parent)
    : DAbstractFileController(parent)
{

}

const DAbstractFileInfoPointer SearchController::createFileInfo(const QSharedPointer<DFMCreateFileInfoEvnet> &event) const
{
    DUrl url = event->url().searchTargetUrl();

    if (url.isSearchFile()) {
        url.setSearchKeyword(event->url().searchKeyword());
    } else {
        url = event->url();
    }

    return DAbstractFileInfoPointer(new SearchFileInfo(url));
}

bool SearchController::openFileLocation(const QSharedPointer<DFMOpenFileLocation> &event) const
{
    return DTK_WIDGET_NAMESPACE::DDesktopServices::showFileItem(realUrl(event->url()));
}

bool SearchController::openFile(const QSharedPointer<DFMOpenFileEvent> &event) const
{
    return DFileService::instance()->openFile(event->sender(), realUrl(event->url()));
}

bool SearchController::openFileByApp(const QSharedPointer<DFMOpenFileByAppEvent> &event) const
{
    return DFileService::instance()->openFileByApp(event->sender(), event->appName(), realUrl(event->url()));
}

bool SearchController::writeFilesToClipboard(const QSharedPointer<DFMWriteUrlsToClipboardEvent> &event) const
{
    return DFileService::instance()->writeFilesToClipboard(event->sender(), event->action(), realUrlList(event->urlList()));
}

DUrlList SearchController::moveToTrash(const QSharedPointer<DFMMoveToTrashEvent> &event) const
{
    return DFileService::instance()->moveToTrash(event->sender(), realUrlList(event->urlList()));
}

bool SearchController::restoreFile(const QSharedPointer<DFMRestoreFromTrashEvent> &event) const
{
    return DFileService::instance()->restoreFile(event->sender(), realUrlList(event->urlList()));
}

bool SearchController::deleteFiles(const QSharedPointer<DFMDeleteEvent> &event) const
{
    return DFileService::instance()->deleteFiles(event->sender(), realUrlList(event->urlList()), false, event->silent(), event->force());
}

bool SearchController::renameFile(const QSharedPointer<DFMRenameEvent> &event) const
{
    return DFileService::instance()->renameFile(event->sender(), realUrl(event->fromUrl()), realUrl(event->toUrl()));
}

bool SearchController::setPermissions(const QSharedPointer<DFMSetPermissionEvent> &event) const
{
    DUrl url = event->url();

    if (!url.searchTargetUrl().isEmpty()) {
        return DFileService::instance()->setPermissions(event->sender(), url.searchTargetUrl(), event->permissions());
    }

    return false;
}

bool SearchController::compressFiles(const QSharedPointer<DFMCompressEvnet> &event) const
{
    return DFileService::instance()->compressFiles(event->sender(), realUrlList(event->urlList()));
}

bool SearchController::decompressFile(const QSharedPointer<DFMDecompressEvnet> &event) const
{
    return DFileService::instance()->decompressFile(event->sender(), realUrlList(event->urlList()));
}

bool SearchController::createSymlink(const QSharedPointer<DFMCreateSymlinkEvent> &event) const
{
    return DFileService::instance()->createSymlink(event->sender(), realUrl(event->fileUrl()), event->toUrl());
}

bool SearchController::shareFolder(const QSharedPointer<DFMFileShareEvnet> &event) const
{
    return DFileService::instance()->shareFolder(event->sender(), realUrl(event->url()),
            event->name(), event->isWritable(), event->allowGuest());
}

bool SearchController::unShareFolder(const QSharedPointer<DFMCancelFileShareEvent> &event) const
{
    return DFileService::instance()->unShareFolder(event->sender(), realUrl(event->url()));
}

bool SearchController::openInTerminal(const QSharedPointer<DFMOpenInTerminalEvent> &event) const
{
    return DFileService::instance()->openInTerminal(event->sender(), realUrl(event->url()));
}

const DDirIteratorPointer SearchController::createDirIterator(const QSharedPointer<DFMCreateDiriterator> &event) const
{
    SearchDiriterator *diriterator = new SearchDiriterator(event->url(), event->nameFilters(),
            event->filters(), event->flags(),
            const_cast<SearchController *>(this));

    return DDirIteratorPointer(diriterator);
}

DAbstractFileWatcher *SearchController::createFileWatcher(const QSharedPointer<DFMCreateFileWatcherEvent> &event) const
{
    if (event->url().searchedFileUrl().isValid()) {
        return 0;
    }

    return new SearchFileWatcher(event->url());
}

bool SearchController::setFileTags(const QSharedPointer<DFMSetFileTagsEvent> &event) const
{
    if (!event->url().searchedFileUrl().isValid()) {
        return false;
    }

    QList<QString> tags = event->tags();
    return fileService->setFileTags(this, event->url().searchedFileUrl(), tags);
}

bool SearchController::removeTagsOfFile(const QSharedPointer<DFMRemoveTagsOfFileEvent> &event) const
{
    if (!event->url().searchedFileUrl().isValid()) {
        return false;
    }

    return fileService->removeTagsOfFile(this, event->url().searchedFileUrl(), event->tags());
}

QList<QString> SearchController::getTagsThroughFiles(const QSharedPointer<DFMGetTagsThroughFilesEvent> &event) const
{
    QList<DUrl> list = event->urlList();
    for (DUrl &item : list) {
        item = item.searchedFileUrl();
    }
    return fileService->getTagsThroughFiles(this, list);
}

DUrl SearchController::realUrl(const DUrl &searchUrl)
{
    return searchUrl.searchedFileUrl();
}

DUrlList SearchController::realUrlList(const DUrlList &searchUrls)
{
    DUrlList list;

    for (const DUrl &url : searchUrls) {
        list << realUrl(url);
    }

    return list;
}
