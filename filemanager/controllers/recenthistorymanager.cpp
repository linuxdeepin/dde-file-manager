#include "recenthistorymanager.h"
#include "fileinfo.h"
#include "desktopfileinfo.h"

#include <QUrl>
#include <QDirIterator>

RecentHistoryManager::RecentHistoryManager(QObject *parent) : BaseManager(parent)
{

}

RecentHistoryManager::~RecentHistoryManager()
{

}

void RecentHistoryManager::load()
{

}

void RecentHistoryManager::save()
{

}

void RecentHistoryManager::fetchFileInformation(const QString &url,
                                                int filter)
{
    const QString path = QUrl(url).path();

    QList<FileInfo*> infolist;

    if(path.isEmpty()) {
        const QFileInfoList list = QDir::drives();

        for(const QFileInfo &info : list) {
            FileInfo *fileInfo = new FileInfo(info);

            infolist.append(fileInfo);
        }
    } else {
        QDirIterator dirIt(path, QDir::Filters(filter));
        FileInfo *fileInfo;

        while (dirIt.hasNext()) {
            dirIt.next();

            if(dirIt.fileInfo().absoluteFilePath() == path)
                continue;

            if(dirIt.fileInfo().suffix() == DESKTOP_SURRIX)
                fileInfo = new DesktopFileInfo(dirIt.fileInfo());
            else
                fileInfo = new FileInfo(dirIt.fileInfo());

            infolist.append(fileInfo);
        }
    }

    emit updates(path, infolist);
}

