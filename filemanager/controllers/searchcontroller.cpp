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

void SearchController::searchStart(const DUrl &fileUrl, QDir::Filters filter)
{
    const DUrl &targetUrl = fileUrl.searchTargetUrl();
    const QString &keyword = fileUrl.searchKeyword();

    QRegularExpression regular(keyword, QRegularExpression::CaseInsensitiveOption);

    QList<DUrl> searchPathList;

    searchPathList << targetUrl;

    qDebug() << "begin search:" << fileUrl;

    while(!searchPathList.isEmpty()) {
        DDirIteratorPointer it = FileServices::instance()->createDirIterator(searchPathList.takeAt(0),
                                                                      QDir::NoDotAndDotDot | filter, QDirIterator::NoIteratorFlags);

        while (it->hasNext()) {
            if(!activeJob.contains(targetUrl)) {
                qDebug() << "stop search:" << fileUrl;

                return;
            }

            AbstractFileInfoPointer fileInfo = FileServices::instance()->createFileInfo(it->next());

            fileInfo->makeAbsolute();

            if(fileInfo->isDir()) {
                searchPathList << fileInfo->fileUrl();
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
