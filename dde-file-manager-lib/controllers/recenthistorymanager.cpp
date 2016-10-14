#include "recenthistorymanager.h"
#include "fileservices.h"

#include "app/filesignalmanager.h"
#include "app/define.h"

#include "models/recentfileinfo.h"
#include "shutil/standardpath.h"

#include "widgets/singleton.h"

#include <QDirIterator>
#include <QJsonObject>
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QDebug>
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
    QString filePath = cachePath();
    QFile file(filePath);
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
    QString filePath = cachePath();
    QFile file(filePath);
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

const QList<AbstractFileInfoPointer> RecentHistoryManager::getChildren(const DUrl &fileUrl, const QStringList &nameFilters,
                                                                       QDir::Filters filters, QDirIterator::IteratorFlags flags,
                                                                       bool &accepted) const
{
    Q_UNUSED(filters)
    Q_UNUSED(fileUrl)
    Q_UNUSED(nameFilters)
    Q_UNUSED(flags)

    QList<AbstractFileInfoPointer> infolist;

    accepted = true;

    for (const DUrl &url : m_openedFileList) {
        RecentFileInfo* info =  new RecentFileInfo(url);
        if (m_lastFileOpenedTime.contains(url)){
            info->setLastOpened(m_lastFileOpenedTime.value(url));
        }
        infolist.append(AbstractFileInfoPointer(info));
    }

    return infolist;
}

const AbstractFileInfoPointer RecentHistoryManager::createFileInfo(const DUrl &fileUrl, bool &accepted) const
{
    accepted = true;

    return AbstractFileInfoPointer(new RecentFileInfo(fileUrl));
}

QString RecentHistoryManager::cachePath()
{
    return getCachePath("recentHistory");
}

void RecentHistoryManager::loadJson(const QJsonObject &json)
{
    m_openedFileList.clear();
    m_lastFileOpenedTime.clear();
    QJsonArray jsonArray = json["RecentHistory"].toArray();
    for(int i = 0; i < jsonArray.size(); i++)
    {
        QJsonObject object = jsonArray[i].toObject();
        QString url = object["url"].toString();
        qint64 mSecsSinceEpoch = object["lastOpened"].toInt();
        DUrl durl = DUrl(url);
        m_openedFileList.append(durl);
        m_lastFileOpenedTime.insert(durl, QDateTime::fromMSecsSinceEpoch(mSecsSinceEpoch));
    }
}

void RecentHistoryManager::writeJson(QJsonObject &json)
{
    QJsonArray localArray;
    for(int i = 0; i < m_openedFileList.size(); i++)
    {
        QJsonObject object;
        object["url"] = m_openedFileList.at(i).toString();
        if (m_lastFileOpenedTime.contains(m_openedFileList.at(i))){
            object["lastOpened"] = m_lastFileOpenedTime.value(m_openedFileList.at(i)).toMSecsSinceEpoch();
        }
        localArray.append(object);
    }
    json["RecentHistory"] = localArray;
}

void RecentHistoryManager::removeRecentFiles(const DUrlList &urlList)
{
    for(const DUrl &url : urlList) {
        m_openedFileList.removeOne(url);
        m_lastFileOpenedTime.remove(url);
        emit childrenRemoved(url);
    }

    save();
}

void RecentHistoryManager::clearRecentFiles()
{
    for(const DUrl &url : m_openedFileList) {
        emit childrenRemoved(url);
    }

    m_openedFileList.clear();
    m_lastFileOpenedTime.clear();
    save();
}

void RecentHistoryManager::addOpenedFile(const DUrl &url)
{
    if(!url.isLocalFile())
        return;

    DUrl recent_url = DUrl::fromRecentFile(url.path());

    m_lastFileOpenedTime.insert(recent_url, QDateTime::currentDateTime());

    if(m_openedFileList.contains(recent_url))
        return;

    m_openedFileList << recent_url;

    emit childrenAdded(recent_url);

    save();
}

