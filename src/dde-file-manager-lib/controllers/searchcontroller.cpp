// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "searchcontroller.h"
#include "dfileservices.h"
#include "dfmevent.h"
#include "dfileproxywatcher.h"
#include "private/dabstractfilewatcher_p.h"

#include "models/searchfileinfo.h"
#include "ddiriterator.h"
#include "shutil/dfmregularexpression.h"
#include "shutil/dfmfilelistfile.h"
#include "dfmapplication.h"
#include "dfmstandardpaths.h"
#include "vaultcontroller.h"
#include "app/define.h"
#include "app/filesignalmanager.h"
#include "models/vaultfileinfo.h"
#include "searchservice.h"

#include <DDesktopServices>

#include <QDebug>
#include <QRegularExpression>
#include <QQueue>
#include <QtConcurrent>

class SearchFileWatcherPrivate;
class SearchFileWatcher : public DAbstractFileWatcher
{
public:
    explicit SearchFileWatcher(const DUrl &url, QObject *parent = nullptr);
    ~SearchFileWatcher() override;

    void setEnabledSubfileWatcher(const DUrl &subfileUrl, bool enabled = true) override;

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
    explicit SearchFileWatcherPrivate(SearchFileWatcher *qq)
        : DAbstractFileWatcherPrivate(qq) {}

    bool start() override;
    bool stop() override;

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
    }
    //这里removeWatcher的逻辑是在文件超出可视区域时，就不监控其变化
    //但这样会导致在一些特殊目录下搜索后，做一些特殊操作（例如最近使用目录中移除文件 、标记目录中取消文件标记等）时，
    //在可视区域外的文件不会从搜索结果中移除
    //宗上，将该行代码注释
    //else {
    //    removeWatcher(subfileUrl.searchedFileUrl());
    //}
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
    DUrl newFromUrl = fileUrl();
    newFromUrl.setSearchedFileUrl(fromUrl);

    DUrl newToUrl = toUrl;
    if (fileUrl().searchTargetUrl().scheme() == toUrl.scheme() && toUrl.path().startsWith(fileUrl().searchTargetUrl().path())) {
        QString keywordPattern = DFMRegularExpression::checkWildcardAndToRegularExpression(fileUrl().searchKeyword());
        const DAbstractFileInfoPointer &info = DFileService::instance()->createFileInfo(this, toUrl);

        QRegularExpression regexp(keywordPattern, QRegularExpression::CaseInsensitiveOption);
        QRegularExpressionMatch match = regexp.match(info->fileDisplayName());
        if (match.hasMatch()) {
            newToUrl = fileUrl();
            newToUrl.setSearchedFileUrl(toUrl);

            /*fix bug34957,搜索模式下，删除文件到回收站的文件不再加入到watcher中，不然会一直删除不掉*/
            if (toUrl.path().contains("/.local/share/Trash/files", Qt::CaseSensitive)) {
                return;
            } else {
                /*fix bug 44187 修改搜索结果名称，文件夹会从搜索结果消失，因为watcher里面增加的是真实路径不是搜索路径*/
                addWatcher(newToUrl);
            }
        }
    }

//    removeWatcher(fromUrl); // fix task 21431 临时解决方案。

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

SearchDiriterator::SearchDiriterator(const DUrl &url, const QStringList &nameFilters,
                                     QDir::Filters filter, QDirIterator::IteratorFlags flags,
                                     SearchController *parent)
    : QObject()
    , DDirIterator()
    , parent(parent)
    , rootUrl(url)
    , keyword(url.searchKeyword())
{
    Q_UNUSED(nameFilters)
    Q_UNUSED(filter)
    Q_UNUSED(flags)

    const DUrl &tmpUrl = url.searchTargetUrl();
    targetUrl = tmpUrl.isComputerFile() ? DUrl::fromLocalFile("/") : tmpUrl;

    initConnect();
    doSearch();
}

SearchDiriterator::~SearchDiriterator()
{

}

void SearchDiriterator::initConnect()
{
    connect(searchServ, &SearchService::matched, this, &SearchDiriterator::onMatched);
    connect(searchServ, &SearchService::searchCompleted, this, &SearchDiriterator::onSearchCompleted);
    connect(searchServ, &SearchService::searchStoped, this, &SearchDiriterator::onSearchStoped);
}

void SearchDiriterator::doSearch()
{
    taskId = QUuid::createUuid().toString(QUuid::WithoutBraces);
    searchServ->search(taskId, targetUrl, keyword);
}

DUrl SearchDiriterator::next()
{
    if (!childrens.isEmpty()) {
        QMutexLocker lk(&mutex);
        const DUrl &url = childrens.dequeue();
        lk.unlock();

        // 修复bug-51754 增加条件判断，保险箱内的文件不能被检索到
        if (VaultController::isVaultFile(url.fragment()) && !VaultController::isVaultFile(targetUrl.toLocalFile()))
            return next();

        currentFileInfo = DFileService::instance()->createFileInfo(parent, url);
        return url;
    }

    return DUrl();
}

bool SearchDiriterator::hasNext() const
{
    if (searchStoped)
        return false;

    QMutexLocker lk(&mutex);
    return !(childrens.isEmpty() && searchCompleted);
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
    return rootUrl;
}

void SearchDiriterator::close()
{
    if (searchCompleted) {
        searchStoped = true;
        return;
    }

    searchServ->stop(taskId);
}

void SearchDiriterator::onMatched(const QString &id)
{
    if (taskId == id) {
        auto results = searchServ->matchedResults(taskId);
        for (const auto &result : results) {
            DUrl url = rootUrl;
            url.setSearchedFileUrl(result);
            QMutexLocker lk(&mutex);
            childrens << url;
        }
    }
}

void SearchDiriterator::onSearchCompleted(const QString &id)
{
    if (taskId == id) {
        qInfo() << "taskId: " << taskId << "search completed!";
        searchCompleted = true;
    }
}

void SearchDiriterator::onSearchStoped(const QString &id)
{
    if (taskId == id)
        searchStoped = true;
}

SearchController::SearchController(QObject *parent)
    : DAbstractFileController(parent)
{

}

const DAbstractFileInfoPointer SearchController::createFileInfo(const QSharedPointer<DFMCreateFileInfoEvent> &event) const
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
    return DFileService::instance()->openFileLocation(event->sender(), realUrl(event->url()));
}

bool SearchController::openFile(const QSharedPointer<DFMOpenFileEvent> &event) const
{
    return DFileService::instance()->openFile(event->sender(), realUrl(event->url()));
}

bool SearchController::openFileByApp(const QSharedPointer<DFMOpenFileByAppEvent> &event) const
{
    return DFileService::instance()->openFileByApp(event->sender(), event->appName(), realUrl(event->url()));
}

bool SearchController::openFilesByApp(const QSharedPointer<DFMOpenFilesByAppEvent> &event) const
{
    return DFileService::instance()->openFilesByApp(event->sender(), event->appName(), realUrlList(event->urlList()), event->isEnter());
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
    /*解决搜索状态下修改文件属性会修改到当前用户的属性*/
    if (!url.searchTargetUrl().isEmpty()) {
        return DFileService::instance()->setPermissions(event->sender(), DUrl(url.fragment()), event->permissions());
    }

    return false;
}

bool SearchController::compressFiles(const QSharedPointer<DFMCompressEvent> &event) const
{
    return DFileService::instance()->compressFiles(event->sender(), realUrlList(event->urlList()));
}

bool SearchController::decompressFile(const QSharedPointer<DFMDecompressEvent> &event) const
{
    return DFileService::instance()->decompressFile(event->sender(), realUrlList(event->urlList()));
}

bool SearchController::addToBookmark(const QSharedPointer<DFMAddToBookmarkEvent> &event) const
{
    return DFileService::instance()->addToBookmark(event->sender(), realUrl(event->url()));
}

bool SearchController::removeBookmark(const QSharedPointer<DFMRemoveBookmarkEvent> &event) const
{
    return DFileService::instance()->removeBookmark(nullptr, realUrl(event->url()));
}

bool SearchController::createSymlink(const QSharedPointer<DFMCreateSymlinkEvent> &event) const
{
    return DFileService::instance()->createSymlink(event->sender(), realUrl(event->fileUrl()), event->toUrl(), event->force());
}

bool SearchController::shareFolder(const QSharedPointer<DFMFileShareEvent> &event) const
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
        return nullptr;
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
