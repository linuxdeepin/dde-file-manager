#include "searchcontroller.h"
#include "fileservices.h"

#include "../models/searchfileinfo.h"

#include "../app/global.h"

#include <QDebug>

SearchController::SearchController(QObject *parent)
    : AbstractFileController(parent)
{
    FileServices::setFileUrlHandler(SEARCH_SCHEME, "", this);
}

const QList<AbstractFileInfo *> SearchController::getChildren(const DUrl &fileUrl, QDir::Filters filter, bool &accepted) const
{
    accepted = true;

    QList<AbstractFileInfo*> list;

    const QString &targetPath = fileUrl.path();
    const QString &keyword = fileUrl.query();

    QStringList nameFilter;

    nameFilter << "*" + keyword + "*";

    QDirIterator it(targetPath, nameFilter, QDir::NoDotAndDotDot | filter, QDirIterator::Subdirectories);

    currentUrl = fileUrl;

    while (it.hasNext()) {
        if(currentUrl != fileUrl)
            return list;

        QThread::msleep(50);

        DUrl url = fileUrl;

        url.setFragment(it.next());

        emit childrenAdded(url);
    }

    return list;
}

AbstractFileInfo *SearchController::createFileInfo(const DUrl &fileUrl, bool &accepted) const
{
    Q_UNUSED(fileUrl)

    accepted = true;

    return new SearchFileInfo(fileUrl);
}
