#include "searchcontroller.h"
#include "dfileservices.h"

#include "models/searchfileinfo.h"
#include "ddiriterator.h"

#include "app/define.h"
#include "app/filesignalmanager.h"

#include <QDebug>
#include <QRegularExpression>
#include <QQueue>

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

private:
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

            it = DFileService::instance()->createDirIterator(url, m_nameFilters,QDir::NoDotAndDotDot | m_filter, m_flags);

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

            if (fileInfo->fileName().indexOf(regular) >= 0) {
                DUrl url = fileUrl;
                const DUrl &realUrl = fileInfo->fileUrl();

                url.setSearchedFileUrl(realUrl);

                if (parent->urlToTargetUrlMap.contains(realUrl, fileUrl)) {
                    ++parent->urlToTargetUrlMapInsertCount[QPair<DUrl, DUrl>(realUrl, fileUrl)];
                } else {
                    parent->urlToTargetUrlMap.insertMulti(realUrl, fileUrl);
                    parent->urlToTargetUrlMapInsertCount[QPair<DUrl, DUrl>(realUrl, fileUrl)] = 0;
                }

                fileService->addUrlMonitor(realUrl);

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

SearchController::SearchController(QObject *parent)
    : DAbstractFileController(parent)
{
    connect(fileService, &DFileService::childrenAdded, this, &SearchController::onFileCreated);
    connect(fileService, &DFileService::childrenRemoved, this, &SearchController::onFileRemove);
}

const DAbstractFileInfoPointer SearchController::createFileInfo(const DUrl &fileUrl, bool &accepted) const
{
    accepted = true;

    return DAbstractFileInfoPointer(new SearchFileInfo(fileUrl));
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

bool SearchController::addUrlMonitor(const DUrl &fileUrl, bool &accepted) const
{
    accepted = true;

    return DFileService::instance()->addUrlMonitor(realUrl(fileUrl));
}

bool SearchController::removeUrlMonitor(const DUrl &url, bool &accepted) const
{
    accepted = true;

    return DFileService::instance()->removeUrlMonitor(realUrl(url));
}

bool SearchController::copyFiles(const DUrlList &urlList, bool &accepted) const
{
    accepted = true;

    return DFileService::instance()->copyFiles(realUrlList(urlList));
}

DUrlList SearchController::moveToTrash(const DUrlList &urlList, bool &accepted) const
{
    accepted = true;

    return DFileService::instance()->moveToTrashSync(realUrlList(urlList));
}

bool SearchController::cutFiles(const DUrlList &urlList, bool &accepted) const
{
    accepted = true;

    return DFileService::instance()->cutFiles(realUrlList(urlList));
}

bool SearchController::deleteFiles(const DUrlList &urlList, const DFMEvent &event, bool &accepted) const
{
    accepted = true;

    return DFileService::instance()->deleteFilesSync(realUrlList(urlList), event);
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

    return DDirIteratorPointer(new SearchDiriterator(fileUrl, nameFilters, filters, flags, const_cast<SearchController*>(this)));
}

void SearchController::onFileCreated(const DUrl &fileUrl)
{
    for (DUrl url : urlToTargetUrlMap.values(fileUrl)) {
        url.setSearchedFileUrl(fileUrl);

        emit childrenAdded(url);
    }
}

void SearchController::onFileRemove(const DUrl &fileUrl)
{
    for (DUrl url : urlToTargetUrlMap.values(fileUrl)) {
        url.setSearchedFileUrl(fileUrl);

        emit childrenRemoved(url);
    }
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
