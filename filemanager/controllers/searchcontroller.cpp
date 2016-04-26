#include "searchcontroller.h"

#include "../models/searchfileinfo.h"

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

    const QString &fragment = fileUrl.fragment();

    if(fragment == "stop") {
        DUrl url = fileUrl;

        url.setFragment(QString());

        activeJob.remove(url);
    } else {
        activeJob << fileUrl;

        QtConcurrent::run(QThreadPool::globalInstance(), const_cast<SearchController*>(this),
                          &SearchController::searchStart, fileUrl, filter);
    }

    return QList<AbstractFileInfoPointer>();
}

AbstractFileInfoPointer SearchController::createFileInfo(const DUrl &fileUrl, bool &accepted) const
{
    accepted = true;

    return AbstractFileInfoPointer(new SearchFileInfo(fileUrl));
}

void SearchController::searchStart(const DUrl &fileUrl, QDir::Filters filter)
{
    const QString &targetPath = fileUrl.path();
    const QString &keyword = fileUrl.query();

    QRegularExpression regular(keyword, QRegularExpression::CaseInsensitiveOption);

    QList<QString> searchPathList;

    searchPathList << targetPath;

    qDebug() << "begin search:" << fileUrl;

    while(!searchPathList.isEmpty()) {
        QDirIterator it(searchPathList.takeAt(0), QDir::NoDotAndDotDot | filter, QDirIterator::NoIteratorFlags);

        while (it.hasNext()) {
            if(!activeJob.contains(fileUrl)) {
                qDebug() << "stop search:" << fileUrl;

                return;
            }

            QFileInfo fileInfo(it.next());

            fileInfo.makeAbsolute();

            if(fileInfo.isDir()) {
                searchPathList << fileInfo.filePath();
            }

            if(fileInfo.fileName().indexOf(regular) >= 0) {
                DUrl url = fileUrl;

                url.setFragment(fileInfo.filePath());

                emit childrenAdded(url);

                QThread::msleep(50);
            }
        }
    }

    qDebug() << "search finished:" << fileUrl;
}
