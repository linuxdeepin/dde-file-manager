#include "recenthistorymanager.h"
#include "fileinfo.h"
#include "desktopfileinfo.h"
#include "../app/global.h"
#include "../app/filesignalmanager.h"

#include <QUrl>
#include <QDirIterator>

RecentHistoryManager::RecentHistoryManager(QObject *parent) : BaseManager(parent)
{
    connect(fileSignalManager, &FileSignalManager::fileOpened,
            this, &RecentHistoryManager::addOpenedFile);
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
                                                int /*filter*/)
{
    QList<FileInfo*> infolist;

    for (const QString &filePath : openedFileList) {
        FileInfo *fileInfo;

        if(filePath.endsWith(DESKTOP_SURRIX))
            fileInfo = new DesktopFileInfo(filePath);
        else
            fileInfo = new FileInfo(filePath);

        infolist.append(fileInfo);
    }

    emit updates(url, infolist);
}

void RecentHistoryManager::addOpenedFile(const QString &url)
{
    if(openedFileList.contains(url))
        return;

    openedFileList << url;

    emit fileCreated(RECENT_ROOT + QFileInfo(url).fileName());
}

