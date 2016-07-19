#include "searchcontroller.h"
#include "fileservices.h"

#include "../models/searchfileinfo.h"
#include "../models/ddiriterator.h"

#include "../app/global.h"
#include "../app/filesignalmanager.h"

#include <QDebug>
#include <QRegularExpression>
#include <QQueue>

class SearchDiriterator : public DDirIterator
{
public:
    SearchDiriterator(const DUrl &url, QDir::Filters filter, QDirIterator::IteratorFlags flags, SearchController *parent);
    ~SearchDiriterator();

    DUrl next() Q_DECL_OVERRIDE;
    bool hasNext() const Q_DECL_OVERRIDE;

    QString fileName() const Q_DECL_OVERRIDE;
    QString filePath() const Q_DECL_OVERRIDE;
    const AbstractFileInfoPointer fileInfo() const Q_DECL_OVERRIDE;
    QString path() const Q_DECL_OVERRIDE;

private:
    SearchController *parent;
    AbstractFileInfoPointer currentFileInfo;
    mutable QQueue<DUrl> childrens;

    DUrl fileUrl;
    DUrl targetUrl;
    QString keyword;
    QRegularExpression regular;
    QDir::Filters m_filter;
    QDirIterator::IteratorFlags m_flags;
    mutable QList<DUrl> searchPathList;
    mutable DDirIteratorPointer it;
};

SearchDiriterator::SearchDiriterator(const DUrl &url, QDir::Filters filter, QDirIterator::IteratorFlags flags, SearchController *parent)
    : DDirIterator()
    , parent(parent)
    , fileUrl(url)
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
        if (!it) {
            if (searchPathList.isEmpty())
                break;

            const DUrl &url = searchPathList.takeAt(0);

            it = FileServices::instance()->createDirIterator(url, QDir::NoDotAndDotDot | m_filter, m_flags);

            if (!it) {
                continue;
            }
        }

        while (it->hasNext()) {
            it->next();

            AbstractFileInfoPointer fileInfo = it->fileInfo();

            fileInfo->makeAbsolute();

            if (fileInfo->isDir()) {
                const DUrl &url = fileInfo->fileUrl();

                if (!searchPathList.contains(url))
                    searchPathList << url;
            }

            if (fileInfo->fileName().indexOf(regular) >= 0) {
                DUrl url = fileUrl;
                const DUrl &realUrl = fileInfo->fileUrl();

                url.setFragment(realUrl.toString());

                if (parent->urlToTargetUrlMap.contains(realUrl, fileUrl)) {
                    ++parent->urlToTargetUrlMapInsertCount[QPair<DUrl, DUrl>(realUrl, fileUrl)];
                } else {
                    parent->urlToTargetUrlMap.insertMulti(fileInfo->fileUrl(), fileUrl);
                    parent->urlToTargetUrlMapInsertCount[QPair<DUrl, DUrl>(realUrl, fileUrl)] = 0;
                }

                fileService->addUrlMonitor(realUrl);

                childrens << url;

                return true;
            }
        }

        it = Q_NULLPTR;
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

const AbstractFileInfoPointer SearchDiriterator::fileInfo() const
{
    return currentFileInfo;
}

QString SearchDiriterator::path() const
{
    return currentFileInfo ? currentFileInfo->path() : QString();
}

SearchController::SearchController(QObject *parent)
    : AbstractFileController(parent)
{
    connect(fileService, &FileServices::childrenAdded, this, &SearchController::onFileCreated);
    connect(fileService, &FileServices::childrenRemoved, this, &SearchController::onFileRemove);
}

const AbstractFileInfoPointer SearchController::createFileInfo(const DUrl &fileUrl, bool &accepted) const
{
    accepted = true;

    return AbstractFileInfoPointer(new SearchFileInfo(fileUrl));
}

bool SearchController::openFileLocation(const DUrl &fileUrl, bool &accepted) const
{
    accepted = true;

    return FileServices::instance()->openFileLocation(realUrl(fileUrl));
}

bool SearchController::openFile(const DUrl &fileUrl, bool &accepted) const
{
    accepted = true;

    return FileServices::instance()->openFile(realUrl(fileUrl));
}

bool SearchController::addUrlMonitor(const DUrl &fileUrl, bool &accepted) const
{
    accepted = true;

    return FileServices::instance()->addUrlMonitor(realUrl(fileUrl));
}

bool SearchController::removeUrlMonitor(const DUrl &url, bool &accepted) const
{
    accepted = true;

    return FileServices::instance()->removeUrlMonitor(realUrl(url));
}

bool SearchController::copyFiles(const DUrlList &urlList, bool &accepted) const
{
    accepted = true;

    return FileServices::instance()->copyFiles(realUrlList(urlList));
}

bool SearchController::moveToTrash(const DUrlList &urlList, bool &accepted) const
{
    accepted = true;

    return FileServices::instance()->moveToTrashSync(realUrlList(urlList));
}

bool SearchController::cutFiles(const DUrlList &urlList, bool &accepted) const
{
    accepted = true;

    return FileServices::instance()->cutFiles(realUrlList(urlList));
}

bool SearchController::deleteFiles(const DUrlList &urlList, const FMEvent &event, bool &accepted) const
{
    accepted = true;

    return FileServices::instance()->deleteFilesSync(realUrlList(urlList), event);
}

bool SearchController::renameFile(const DUrl &oldUrl, const DUrl &newUrl, bool &accepted) const
{
    accepted = true;

    return FileServices::instance()->renameFile(realUrl(oldUrl), realUrl(newUrl));
}

bool SearchController::compressFiles(const DUrlList &urlList, bool &accepted) const
{
    accepted = true;

    return FileServices::instance()->compressFiles(realUrlList(urlList));
}

bool SearchController::decompressFile(const DUrl &fileUrl, bool &accepted) const
{
    accepted = true;

    return FileServices::instance()->decompressFile(realUrl(fileUrl));
}

bool SearchController::createSymlink(const DUrl &fileUrl, const DUrl &linkToUrl, bool &accepted) const
{
    accepted = true;

    return FileServices::instance()->createSymlink(realUrl(fileUrl), linkToUrl);
}

bool SearchController::openInTerminal(const DUrl &fileUrl, bool &accepted) const
{
    accepted = true;

    return FileServices::instance()->openInTerminal(realUrl(fileUrl));
}

const DDirIteratorPointer SearchController::createDirIterator(const DUrl &fileUrl, QDir::Filters filters,
                                                              QDirIterator::IteratorFlags flags, bool &accepted) const
{
    accepted = true;

    return DDirIteratorPointer(new SearchDiriterator(fileUrl, filters, flags, const_cast<SearchController*>(this)));
}

void SearchController::onFileCreated(const DUrl &fileUrl)
{
    for (DUrl url : urlToTargetUrlMap.values(fileUrl)) {
        url.setFragment(fileUrl.toString());

        emit childrenAdded(url);
    }
}

void SearchController::onFileRemove(const DUrl &fileUrl)
{
    for (DUrl url : urlToTargetUrlMap.values(fileUrl)) {
        url.setFragment(fileUrl.toString());

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
    return DUrl(searchUrl.fragment());
}

DUrlList SearchController::realUrlList(const DUrlList &searchUrls)
{
    DUrlList list;

    for (const DUrl &url : searchUrls) {
        list << realUrl(url);
    }

    return list;
}
