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
    load();
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

void RecentHistoryManager::loadJson(const QJsonObject &json)
{
    QJsonArray jsonArray = json["RecentHistory"].toArray();
    for(int i = 0; i < jsonArray.size(); i++)
    {
        QJsonObject object = jsonArray[i].toObject();
        QString url = object["url"].toString();
        openedFileList.append(url);
    }
}

void RecentHistoryManager::writeJson(QJsonObject &json)
{
    QJsonArray localArray;
    for(int i = 0; i < openedFileList.size(); i++)
    {
        QJsonObject object;
        object["url"] = openedFileList.at(i);
        localArray.append(object);
    }
    json["RecentHistory"] = localArray;
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

    save();
}

