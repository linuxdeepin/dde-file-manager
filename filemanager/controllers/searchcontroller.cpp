#include "searchcontroller.h"
#include "fileservices.h"

#include "../models/searchfileinfo.h"

#include "../app/global.h"

#include <QDebug>
#include <QtConcurrent/QtConcurrentRun>

SearchController::SearchController(QObject *parent)
    : AbstractFileController(parent)
{
    FileServices::setFileUrlHandler(SEARCH_SCHEME, "", this);
}

const QList<AbstractFileInfo*> SearchController::getChildren(const DUrl &fileUrl, QDir::Filters filter, bool &accepted) const
{
    accepted = true;

    const QString &fragment = fileUrl.fragment();

    qDebug() << fragment;

    if(fragment == "stop") {
        DUrl url = fileUrl;

        url.setFragment(QString());

        activeJob.remove(url);
    } else {
        activeJob << fileUrl;

        QtConcurrent::run(QThreadPool::globalInstance(), const_cast<SearchController*>(this),
                          &SearchController::searchStart, fileUrl, filter);
    }

    return QList<AbstractFileInfo*>();
}

AbstractFileInfo *SearchController::createFileInfo(const DUrl &fileUrl, bool &accepted) const
{
    accepted = true;

    return new SearchFileInfo(fileUrl);
}

void SearchController::searchStart(const DUrl &fileUrl, QDir::Filters filter)
{
    const QString &targetPath = fileUrl.path();
    const QString &keyword = fileUrl.query();

    QStringList nameFilter;

    nameFilter << "*" + keyword + "*";

    QDirIterator it(targetPath, nameFilter, QDir::NoDotAndDotDot | filter, QDirIterator::Subdirectories);

    while (it.hasNext()) {
        if(!activeJob.contains(fileUrl)) {
            return;
        }

        QThread::msleep(50);

        DUrl url = fileUrl;

        url.setFragment(it.next());

        emit childrenAdded(url);
    }
}
