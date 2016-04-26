#include "recenthistorymanager.h"
#include "recentfileinfo.h"

#include "../app/filesignalmanager.h"

#include <QDirIterator>
#include <QJsonObject>
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QByteArray>
#include <QDateTime>
#include <QUrl>

#define fileService FileServices::instance()

RecentHistoryManager *firstRecent = Q_NULLPTR;

RecentHistoryManager::RecentHistoryManager(QObject *parent)
    : AbstractFileController(parent)
    , BaseManager()
{
    if(!firstRecent) {
        firstRecent = this;

        connect(fileService, &FileServices::fileOpened,
                this, &RecentHistoryManager::addOpenedFile);
        connect(fileSignalManager, &FileSignalManager::requestRecentFileRemove,
                this, &RecentHistoryManager::removeRecentFiles);
        connect(fileSignalManager, &FileSignalManager::requestClearRecent,
                this, &RecentHistoryManager::clearRecentFiles);

        RecentHistoryManager::load();
    }
}

RecentHistoryManager::~RecentHistoryManager()
{

}

void RecentHistoryManager::load()
{
    //TODO: check permission and existence of the path
    QString user = getenv("USER");
    QString configPath = "/home/" + user + "/.cache/dde-file-manager/recentHistory.json";
    QFile file(configPath);
    if (!file.open(QIODevice::ReadOnly))
    {
        qDebug() << "Couldn't open recent history file!";
        return;
    }
    QByteArray data = file.readAll();
    QJsonDocument jsonDoc(QJsonDocument::fromJson(data));
    loadJson(jsonDoc.object());
}

void RecentHistoryManager::save()
{
    //TODO: check permission and existence of the path
    QString user = getenv("USER");
    QString configPath = "/home/" + user + "/.cache/dde-file-manager/recentHistory.json";
    QFile file(configPath);
    if (!file.open(QIODevice::WriteOnly))
    {
        qDebug() << "Couldn't open recent history file!";
        return;
    }
    QJsonObject object;
    writeJson(object);
    QJsonDocument jsonDoc(object);
    file.write(jsonDoc.toJson());
}

bool RecentHistoryManager::openFile(const DUrl &fileUrl, bool &accepted) const
{
    accepted = true;

    return fileService->openFile(DUrl::fromLocalFile(fileUrl.path()));
}

bool RecentHistoryManager::copyFiles(const DUrlList &urlList, bool &accepted) const
{
    accepted = true;

    DUrlList localList;

    for(const DUrl &url : urlList) {
        localList << DUrl::fromLocalFile(url.path());
    }

    return FileServices::instance()->copyFiles(localList);
}

const QList<AbstractFileInfoPointer> RecentHistoryManager::getChildren(const DUrl &fileUrl, QDir::Filters filter, bool &accepted) const
{
    Q_UNUSED(filter)

    QList<AbstractFileInfoPointer> infolist;

    if(fileUrl.path() != "/") {
        accepted = false;

        return infolist;
    }

    accepted = true;

    for (const DUrl &url : openedFileList) {
        infolist.append(AbstractFileInfoPointer(new RecentFileInfo(url)));
    }

    return infolist;
}

AbstractFileInfoPointer RecentHistoryManager::createFileInfo(const DUrl &fileUrl, bool &accepted) const
{
    accepted = true;

    return AbstractFileInfoPointer(new RecentFileInfo(fileUrl));
}

void RecentHistoryManager::loadJson(const QJsonObject &json)
{
    QJsonArray jsonArray = json["RecentHistory"].toArray();
    for(int i = 0; i < jsonArray.size(); i++)
    {
        QJsonObject object = jsonArray[i].toObject();
        QString url = object["url"].toString();
        openedFileList.append(DUrl(url));
    }
}

void RecentHistoryManager::writeJson(QJsonObject &json)
{
    QJsonArray localArray;
    for(int i = 0; i < openedFileList.size(); i++)
    {
        QJsonObject object;
        object["url"] = openedFileList.at(i).toString();
        localArray.append(object);
    }
    json["RecentHistory"] = localArray;
}

void RecentHistoryManager::removeRecentFiles(const DUrlList &urlList)
{
    for(const DUrl &url : urlList) {
        openedFileList.removeOne(url);

        emit childrenRemoved(url);
    }

    save();
}

void RecentHistoryManager::clearRecentFiles()
{
    for(const DUrl &url : openedFileList) {
        emit childrenRemoved(url);
    }

    openedFileList.clear();

    save();
}

void RecentHistoryManager::addOpenedFile(const DUrl &url)
{
    if(!url.isLocalFile())
        return;

    DUrl recent_url = DUrl::fromRecentFile(url.path());

    if(openedFileList.contains(recent_url))
        return;

    openedFileList << recent_url;

    emit childrenAdded(recent_url);

    save();
}

