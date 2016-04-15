#include "recenthistorymanager.h"
#include "recentfileinfo.h"

#include "../app/filesignalmanager.h"

#include "../controllers/fileservices.h"

#include <QDirIterator>
#include <QJsonObject>
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QByteArray>
#include <QDateTime>
#include <QUrl>

#define fileService FileServices::instance()

RecentHistoryManager::RecentHistoryManager(QObject *parent)
    : AbstractFileController(parent)
    , BaseManager()
{
    connect(fileService, &FileServices::fileOpened,
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

bool RecentHistoryManager::openFile(const DUrl &fileUrl, bool &accepted) const
{
    accepted = true;

    return fileService->openFile(DUrl::fromLocalFile(fileUrl.path()));
}

const QList<AbstractFileInfo *> RecentHistoryManager::getChildren(const DUrl &fileUrl, QDir::Filters filter, bool &accepted) const
{
    Q_UNUSED(filter)

    QList<AbstractFileInfo*> infolist;

    if(fileUrl.path() != "/") {
        accepted = false;

        return infolist;
    }

    for (const DUrl &fileUrl : openedFileList) {
        DUrl url(fileUrl);

        url.setScheme(RECENT_SCHEME);

        AbstractFileInfo *fileInfo = new RecentFileInfo(url);

        infolist.append(fileInfo);
    }

    accepted = true;

    return infolist;
}

AbstractFileInfo *RecentHistoryManager::createFileInfo(const DUrl &fileUrl, bool &accepted) const
{
    accepted = true;

    return new RecentFileInfo(fileUrl);
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

void RecentHistoryManager::addOpenedFile(const DUrl &url)
{
    if(openedFileList.contains(url))
        return;

    openedFileList << url;


    //emit fileCreated(RECENT_ROOT + QFileInfo(url).fileName());

    save();
}

