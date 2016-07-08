#include "searchcontroller.h"
#include "fileservices.h"

#include "../models/searchfileinfo.h"
#include "../models/ddiriterator.h"
#include "../app/global.h"
#include "../app/filesignalmanager.h"
#include <QDebug>
#include <QtConcurrent/QtConcurrentRun>
#include <QDirIterator>
#include <QRegularExpression>

SearchController::SearchController(QObject *parent)
    : AbstractFileController(parent)
{
    connect(fileService, &FileServices::childrenAdded, this, &SearchController::onFileCreated);
    connect(fileService, &FileServices::childrenRemoved, this, &SearchController::onFileRemove);
}

const QList<AbstractFileInfoPointer> SearchController::getChildren(const DUrl &fileUrl, QDir::Filters filter, const FMEvent &event, bool &accepted) const
{
    accepted = true;

    if (fileUrl.isStopSearch()) {
        DUrl url = fileUrl;

        url.setSearchAction(DUrl::StartSearch);

        const_cast<SearchController*>(this)->removeJob(url);
    } else {
        activeJob << fileUrl;

        QtConcurrent::run(QThreadPool::globalInstance(), const_cast<SearchController*>(this),
                          &SearchController::searchStart, fileUrl, filter, event);
    }

    return QList<AbstractFileInfoPointer>();
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

    return FileServices::instance()->renameFile(realUrl(oldUrl), DUrl::fromLocalFile(newUrl.path()));
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

void SearchController::searchStart(const DUrl &fileUrl, QDir::Filters filter, const FMEvent &event)
{
    const DUrl &targetUrl = fileUrl.searchTargetUrl();
    const QString &keyword = fileUrl.searchKeyword();

    QRegularExpression regular(keyword, QRegularExpression::CaseInsensitiveOption);

    QList<DUrl> searchPathList;

    searchPathList << targetUrl;

    qDebug() << "begin search:" << fileUrl;

    emit fileSignalManager->searchingIndicatorShowed(event, true);

    while(!searchPathList.isEmpty()) {
        const DUrl &url = searchPathList.takeAt(0);

        DDirIteratorPointer it = FileServices::instance()->createDirIterator(url, QDir::NoDotAndDotDot | filter,
                                                                             QDirIterator::NoIteratorFlags);

        if (!it) {
            searchPathList.removeOne(url);

            continue;
        }

        while (it->hasNext()) {
            if (!activeJob.contains(fileUrl)) {
                qDebug() << "stop search:" << fileUrl;

                removeJob(targetUrl);

                emit fileSignalManager->searchingIndicatorShowed(event, false);

                return;
            }

            AbstractFileInfoPointer fileInfo = FileServices::instance()->createFileInfo(it->next());

            fileInfo->makeAbsolute();

            if(fileInfo->isDir()) {
                const DUrl &url = fileInfo->fileUrl();

                if (!searchPathList.contains(url))
                    searchPathList << url;
            }

            if(fileInfo->fileName().indexOf(regular) >= 0) {
                DUrl url = fileUrl;
                const DUrl &realUrl = fileInfo->fileUrl();

                url.setFragment(realUrl.toString());

                if (urlToTargetUrlMap.contains(realUrl, fileUrl)) {
                    ++urlToTargetUrlMapInsertCount[QPair<DUrl, DUrl>(realUrl, fileUrl)];
                } else {
                    urlToTargetUrlMap.insertMulti(fileInfo->fileUrl(), fileUrl);
                    urlToTargetUrlMapInsertCount[QPair<DUrl, DUrl>(realUrl, fileUrl)] = 0;
                }

                fileService->addUrlMonitor(realUrl);

                emit childrenAdded(url);

                QThread::msleep(50);
            }
        }
    }

    removeJob(targetUrl);

    emit fileSignalManager->searchingIndicatorShowed(event, false);
    qDebug() << "search finished:" << fileUrl;
}

void SearchController::removeJob(const DUrl &fileUrl)
{
    activeJob.remove(fileUrl);

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
