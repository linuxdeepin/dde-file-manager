#include "searchcontroller.h"
#include "dfileservices.h"
#include "dfmevent.h"
#include "dfileproxywatcher.h"
#include "private/dabstractfilewatcher_p.h"

#include "models/searchfileinfo.h"
#include "ddiriterator.h"

#include "app/define.h"
#include "app/filesignalmanager.h"

#include <QDebug>
#include <QRegularExpression>
#include <QQueue>

class SearchFileWatcherPrivate;
class SearchFileWatcher : public DAbstractFileWatcher
{
public:
    explicit SearchFileWatcher(const DUrl &url, QObject *parent = 0);
    ~SearchFileWatcher();

    void addWatcher(const DUrl &url);
    void removeWatcher(const DUrl &url);

private:
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

    QMap<DUrl, DAbstractFileWatcher*> urlToWatcherMap;

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
    SearchController::urlToSearchFileWatcher.remove(fileUrl());
}

void SearchFileWatcher::addWatcher(const DUrl &url)
{
    Q_D(SearchFileWatcher);

    if (!url.isValid() || d->urlToWatcherMap.contains(url))
        return;

    DAbstractFileWatcher *watcher = DFileService::instance()->createFileWatcher(url, this);

    if (!watcher)
        return;

    watcher->moveToThread(this->thread());
    watcher->setParent(this);

    d->urlToWatcherMap[url] = watcher;

    connect(watcher, &DAbstractFileWatcher::fileAttributeChanged, this, &SearchFileWatcher::onFileAttributeChanged);
    connect(watcher, &DAbstractFileWatcher::fileDeleted, this, &SearchFileWatcher::onFileDeleted);
    connect(watcher, &DAbstractFileWatcher::fileModified, this, &SearchFileWatcher::onFileModified);
    connect(watcher, &DAbstractFileWatcher::fileMoved, this, &SearchFileWatcher::onFileMoved);

    if (d->started)
        watcher->startWatcher();
}

void SearchFileWatcher::removeWatcher(const DUrl &url)
{
    Q_D(SearchFileWatcher);

    DAbstractFileWatcher *watcher = d->urlToWatcherMap.value(url);

    if (!watcher)
        return;

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

    DUrl newToUrl = fileUrl();
    newToUrl.setSearchedFileUrl(toUrl);

    removeWatcher(fromUrl);
    addWatcher(toUrl);

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

    for (DAbstractFileWatcher *watcher : urlToWatcherMap)
        ok = ok && watcher->startWatcher();

    return ok;
}

bool SearchFileWatcherPrivate::stop()
{
    bool ok = true;

    for (DAbstractFileWatcher *watcher : urlToWatcherMap)
        ok = ok && watcher->stopWatcher();

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
    QString filePath() const Q_DECL_OVERRIDE;
    const DAbstractFileInfoPointer fileInfo() const Q_DECL_OVERRIDE;
    QString path() const Q_DECL_OVERRIDE;
    void close() Q_DECL_OVERRIDE;

    SearchController *parent;
    DAbstractFileInfoPointer currentFileInfo;
    mutable QQueue<DUrl> childrens;

    DUrl fileUrl;
    DUrl targetUrl;
    QString keyword;
    QRegularExpression regular;
    QStringList m_nameFilters;
    QDir::Filters m_filter;
    QDirIterator::IteratorFlags m_flags;
    mutable QList<DUrl> searchPathList;
    mutable DDirIteratorPointer it;

    SearchFileWatcher *fileWatcher = Q_NULLPTR;

    bool closed = false;
};

SearchDiriterator::SearchDiriterator(const DUrl &url, const QStringList &nameFilters,
                                     QDir::Filters filter, QDirIterator::IteratorFlags flags,
                                     SearchController *parent)
    : DDirIterator()
    , parent(parent)
    , fileUrl(url)
    , m_nameFilters(nameFilters)
    , m_filter(filter)
    , m_flags(flags)
    , fileWatcher(new SearchFileWatcher(url))
{
    targetUrl = url.searchTargetUrl();
    keyword = url.searchKeyword();
    regular = QRegularExpression(QRegularExpression::escape((keyword)), QRegularExpression::CaseInsensitiveOption);
    searchPathList << targetUrl;
}

SearchDiriterator::~SearchDiriterator()
{
    parent->removeJob(targetUrl);
}

DUrl SearchDiriterator::next()
{
    if (!childrens.isEmpty()) {
        const DUrl &url = childrens.dequeue();
        bool accpeted;

        currentFileInfo = parent->createFileInfo(url, accpeted);

        if (fileWatcher)
            fileWatcher->addWatcher(url.searchedFileUrl());

        return url;
    }

    return DUrl();
}

bool SearchDiriterator::hasNext() const
{
    if (!childrens.isEmpty())
        return true;

    forever {
        if (closed)
            return false;

        if (!it) {
            if (searchPathList.isEmpty())
                break;

            const DUrl &url = searchPathList.takeAt(0);

            it = DFileService::instance()->createDirIterator(url, m_nameFilters, QDir::NoDotAndDotDot | m_filter, m_flags);

            if (!it) {
                continue;
            }
        }

        while (it->hasNext()) {
            if (closed)
                return false;

            it->next();

            DAbstractFileInfoPointer fileInfo = it->fileInfo();

            fileInfo->makeAbsolute();

            if (fileInfo->isDir()) {
                const DUrl &url = fileInfo->fileUrl();

                if (!searchPathList.contains(url))
                    searchPathList << url;
            }

            if (fileInfo->fileDisplayName().indexOf(regular) >= 0) {
                DUrl url = fileUrl;
                const DUrl &realUrl = fileInfo->fileUrl();

                url.setSearchedFileUrl(realUrl);

                if (parent->urlToTargetUrlMap.contains(realUrl, fileUrl)) {
                    ++parent->urlToTargetUrlMapInsertCount[QPair<DUrl, DUrl>(realUrl, fileUrl)];
                } else {
                    parent->urlToTargetUrlMap.insertMulti(realUrl, fileUrl);
                    parent->urlToTargetUrlMapInsertCount[QPair<DUrl, DUrl>(realUrl, fileUrl)] = 0;
                }

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

QString SearchDiriterator::filePath() const
{
    return currentFileInfo ? currentFileInfo->filePath() : QString();
}

const DAbstractFileInfoPointer SearchDiriterator::fileInfo() const
{
    return currentFileInfo;
}

QString SearchDiriterator::path() const
{
    return currentFileInfo ? currentFileInfo->path() : QString();
}

void SearchDiriterator::close()
{
    closed = true;
}

QMap<DUrl, SearchFileWatcher*> SearchController::urlToSearchFileWatcher;

SearchController::SearchController(QObject *parent)
    : DAbstractFileController(parent)
{

}

const DAbstractFileInfoPointer SearchController::createFileInfo(const DUrl &fileUrl, bool &accepted) const
{
    accepted = true;

    DUrl url = fileUrl.searchTargetUrl();

    if (url.isSearchFile()) {
        url.setSearchKeyword(fileUrl.searchKeyword());
    } else {
        url = fileUrl;
    }

    return DAbstractFileInfoPointer(new SearchFileInfo(url));
}

bool SearchController::openFileLocation(const DUrl &fileUrl, bool &accepted) const
{
    accepted = true;

    return DFileService::instance()->openFileLocation(realUrl(fileUrl));
}

bool SearchController::openFile(const DUrl &fileUrl, bool &accepted) const
{
    accepted = true;

    return DFileService::instance()->openFile(realUrl(fileUrl));
}

bool SearchController::openFileByApp(const DUrl &fileUrl, const QString &app, bool &accepted) const
{
    accepted = true;
    return DFileService::instance()->openFileByApp(realUrl(fileUrl), app);
}

bool SearchController::copyFilesToClipboard(const DUrlList &urlList, bool &accepted) const
{
    accepted = true;

    return DFileService::instance()->copyFilesToClipboard(realUrlList(urlList));
}

DUrlList SearchController::moveToTrash(const DFMEvent &event, bool &accepted) const
{
    accepted = true;

    const_cast<DFMEvent&>(event) << event.fileUrl().searchTargetUrl();
    const_cast<DFMEvent&>(event) << realUrlList(event.fileUrlList());

    return DFileService::instance()->moveToTrashSync(event);
}

bool SearchController::cutFilesToClipboard(const DUrlList &urlList, bool &accepted) const
{
    accepted = true;

    return DFileService::instance()->cutFilesToClipboard(realUrlList(urlList));
}

bool SearchController::deleteFiles(const DFMEvent &event, bool &accepted) const
{
    accepted = true;

    const_cast<DFMEvent&>(event) << realUrlList(event.fileUrlList());

    return DFileService::instance()->deleteFilesSync(event);
}

bool SearchController::renameFile(const DUrl &oldUrl, const DUrl &newUrl, bool &accepted) const
{
    accepted = true;

    return DFileService::instance()->renameFile(realUrl(oldUrl), realUrl(newUrl));
}

bool SearchController::compressFiles(const DUrlList &urlList, bool &accepted) const
{
    accepted = true;

    return DFileService::instance()->compressFiles(realUrlList(urlList));
}

bool SearchController::decompressFile(const DUrlList &fileUrlList, bool &accepted) const
{
    accepted = true;

    return DFileService::instance()->decompressFile(realUrlList(fileUrlList));
}

bool SearchController::createSymlink(const DUrl &fileUrl, const DUrl &linkToUrl, bool &accepted) const
{
    accepted = true;

    return DFileService::instance()->createSymlink(realUrl(fileUrl), linkToUrl);
}

bool SearchController::unShareFolder(const DUrl &fileUrl, bool &accepted) const
{
    accepted = true;

    return DFileService::instance()->unShareFolder(realUrl(fileUrl));
}

bool SearchController::openInTerminal(const DUrl &fileUrl, bool &accepted) const
{
    accepted = true;

    return DFileService::instance()->openInTerminal(realUrl(fileUrl));
}

const DDirIteratorPointer SearchController::createDirIterator(const DUrl &fileUrl, const QStringList &nameFilters,
                                                              QDir::Filters filters, QDirIterator::IteratorFlags flags,
                                                              bool &accepted) const
{
    accepted = true;

    SearchDiriterator *diriterator = new SearchDiriterator(fileUrl, nameFilters, filters, flags, const_cast<SearchController*>(this));

    diriterator->fileWatcher = urlToSearchFileWatcher.value(fileUrl);

    return DDirIteratorPointer(diriterator);
}

DAbstractFileWatcher *SearchController::createFileWatcher(const DUrl &fileUrl, QObject *parent, bool &accepted) const
{
    Q_UNUSED(parent)

    accepted = !fileUrl.searchedFileUrl().isValid();

    if (!accepted)
        return 0;

    SearchFileWatcher *watcher = new SearchFileWatcher(fileUrl, parent);

    urlToSearchFileWatcher[fileUrl] = watcher;

    return watcher;
}

void SearchController::removeJob(const DUrl &fileUrl)
{
    for (const DUrl &url : urlToTargetUrlMap.keys(fileUrl)) {
        const QPair<DUrl, DUrl> &key = QPair<DUrl, DUrl>(url, fileUrl);
        int count = urlToTargetUrlMapInsertCount.value(key, 0);

        if (--count < 0) {
            urlToTargetUrlMap.remove(url, fileUrl);

            urlToTargetUrlMapInsertCount.remove(key);
        } else {
            urlToTargetUrlMapInsertCount[key] = count;
        }
    }

    urlToTargetUrlMap.remove(fileUrl);
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
