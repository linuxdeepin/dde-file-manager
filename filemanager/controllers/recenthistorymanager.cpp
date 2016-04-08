#include "recenthistorymanager.h"
#include "recentfileinfo.h"

#include "../app/global.h"
#include "../app/filesignalmanager.h"

#include <QDirIterator>
#include <QJsonObject>
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QByteArray>
#include <QDateTime>
#include <QUrl>

#include <stdlib.h>

RecentHistoryManager::RecentHistoryManager(QObject *parent)
    : AbstractFileController(parent)
    , BaseManager()
{
    connect(fileSignalManager, &FileSignalManager::fileOpened,
            this, &RecentHistoryManager::addOpenedFile);

    RecentHistoryManager::load();

    FileServices::setFileUrlHandler(RECENT_SCHEME, "", this);
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

bool RecentHistoryManager::openFile(const QString &fileUrl, bool &accepted) const
{
    QUrl url(fileUrl);

    accepted = false;

    if(url.scheme() != RECENT_SCHEME) {
        return false;
    }

    return fileService->openFile("file://" + url.path());
}

const QList<AbstractFileInfo *> RecentHistoryManager::getChildren(const QString &fileUrl, QDir::Filters filter, bool &accepted) const
{
    Q_UNUSED(filter)

    QUrl url(fileUrl);

    QList<AbstractFileInfo*> infolist;

    if(url.path() != "/" || url.scheme() != RECENT_SCHEME) {
        accepted = false;

        return infolist;
    }

    for (const QString &filePath : openedFileList) {
        QUrl url(filePath);

        url.setScheme(RECENT_SCHEME);

        AbstractFileInfo *fileInfo = new RecentFileInfo(url.toString());

        infolist.append(fileInfo);
    }

    accepted = true;

    return infolist;
}

AbstractFileInfo *RecentHistoryManager::createFileInfo(const QString &fileUrl, bool &accepted) const
{
    QUrl url(fileUrl);

    if(url.scheme() != RECENT_SCHEME) {
        accepted = false;

        return Q_NULLPTR;
    }

    accepted = true;

    return new RecentFileInfo(url.toString());
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

void RecentHistoryManager::addOpenedFile(const QString &url)
{
    if(openedFileList.contains(url))
        return;

    openedFileList << url;


    //emit fileCreated(RECENT_ROOT + QFileInfo(url).fileName());

    save();
}

