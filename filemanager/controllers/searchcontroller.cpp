#include "searchcontroller.h"
#include "fileservices.h"

#include "../models/searchfileinfo.h"
#include "../models/ddiriterator.h"

#include <QDebug>
#include <QtConcurrent/QtConcurrentRun>
#include <QDirIterator>
#include <QRegularExpression>

SearchController::SearchController(QObject *parent)
    : AbstractFileController(parent)
{

}

const QList<AbstractFileInfoPointer> SearchController::getChildren(const DUrl &fileUrl, QDir::Filters filter, bool &accepted) const
{
    accepted = true;

    if(fileUrl.isStopSearch()) {
        activeJob.remove(fileUrl.searchTargetUrl());
    } else {
        activeJob << fileUrl.searchTargetUrl();

        QtConcurrent::run(QThreadPool::globalInstance(), const_cast<SearchController*>(this),
                          &SearchController::searchStart, fileUrl, filter);
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

void SearchController::searchStart(const DUrl &fileUrl, QDir::Filters filter)
{
    const DUrl &targetUrl = fileUrl.searchTargetUrl();
    const QString &keyword = fileUrl.searchKeyword();

    QRegularExpression regular(keyword, QRegularExpression::CaseInsensitiveOption);

    QList<DUrl> searchPathList;

    searchPathList << targetUrl;

    qDebug() << "begin search:" << fileUrl;

    while(!searchPathList.isEmpty()) {
        const DUrl &url = searchPathList.takeAt(0);

        DDirIteratorPointer it = FileServices::instance()->createDirIterator(url, QDir::NoDotAndDotDot | filter,
                                                                             QDirIterator::NoIteratorFlags);

        if (!it) {
            searchPathList.removeOne(url);

            continue;
        }

        while (it->hasNext()) {
            if(!activeJob.contains(targetUrl)) {
                qDebug() << "stop search:" << fileUrl;

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

                url.setFragment(fileInfo->fileUrl().toString());

                emit childrenAdded(url);

                QThread::msleep(50);
            }
        }
    }

    qDebug() << "search finished:" << fileUrl;
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
