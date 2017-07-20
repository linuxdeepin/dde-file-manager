#include "bookmarkmanager.h"
#include "dfileservices.h"
#include "dfmevent.h"
#include "dabstractfilewatcher.h"

#include "app/define.h"

#include <QJsonObject>
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QByteArray>
#include <QDateTime>
#include <QUrl>

#include <stdlib.h>

#include "shutil/fileutils.h"
#include "deviceinfo/udisklistener.h"


BookMarkManager::BookMarkManager(QObject *parent)
    : DAbstractFileController(parent)
    , BaseManager()
{
    load();
    fileService->setFileUrlHandler(BOOKMARK_SCHEME, "", this);

    DAbstractFileWatcher *bookmark_watcher = DFileService::instance()->createFileWatcher(this, DUrl::fromLocalFile(cachePath()), this);

    connect(bookmark_watcher, &DAbstractFileWatcher::fileAttributeChanged, this, &BookMarkManager::reLoad);
    connect(bookmark_watcher, &DAbstractFileWatcher::fileModified, this, &BookMarkManager::reLoad);

    bookmark_watcher->startWatcher();
}

BookMarkManager::~BookMarkManager()
{

}

void BookMarkManager::load()
{
    //Migration for old config files, and rmove that codes for further
    FileUtils::migrateConfigFileFromCache("bookmark");

    //TODO: check permission and existence of the path
    QString configPath = cachePath();
    QFile file(configPath);
    if (!file.open(QIODevice::ReadOnly))
    {
        qDebug() << "Couldn't open bookmark file!";
        return;
    }
    QByteArray data = file.readAll();
    QJsonDocument jsonDoc(QJsonDocument::fromJson(data));
    loadJson(jsonDoc.object());
    file.close();
}

void BookMarkManager::save()
{
    //TODO: check permission and existence of the path
    QString configPath = cachePath();
    QFile file(configPath);
    if (!file.open(QIODevice::WriteOnly))
    {
        qDebug() << "Couldn't write bookmark file!";
        return;
    }
    QJsonObject object;
    writeJson(object);
    QJsonDocument jsonDoc(object);
    file.write(jsonDoc.toJson());
    file.close();
}

QList<BookMarkPointer> BookMarkManager::getBookmarks()
{
    return m_bookmarks;
}

QString BookMarkManager::cachePath()
{
    return getConfigPath("bookmark");
}

void BookMarkManager::loadJson(const QJsonObject &json)
{
    QJsonArray jsonArray = json["Bookmark"].toArray();
    for(int i = 0; i < jsonArray.size(); i++)
    {
        QJsonObject object = jsonArray[i].toObject();
        QString time = object["t"].toString();
        QString name = object["n"].toString();
        QString url = object["u"].toString();

        QString deviceID = object["deviceID"].toString();
        if (deviceID.isEmpty()){
            DFileInfo info(url);
            deviceID = info.getDiskinfo().id();
        }
        BookMarkPointer bm(new BookMark(QDateTime::fromString(time), name, DUrl(url)));
        bm->setDevcieId(deviceID);
        m_bookmarks.append(bm);
    }
}

void BookMarkManager::writeJson(QJsonObject &json)
{
    QJsonArray localArray;
    for(int i = 0; i < m_bookmarks.size(); i++)
    {
        QJsonObject object;
        object["t"] = m_bookmarks.at(i)->getDateTime().toString();
        object["n"] = m_bookmarks.at(i)->getName();
        object["u"] = m_bookmarks.at(i)->getUrl().toString();

        QString deviceID = m_bookmarks.at(i)->getDiskinfo().id();
        if (deviceID.isEmpty()){
            DFileInfo info(m_bookmarks.at(i)->getUrl());
            deviceID = info.getDiskinfo().id();
        }
        object["deviceID"] = deviceID;
        localArray.append(object);
    }
    json["Bookmark"] = localArray;
}

BookMarkPointer BookMarkManager::writeIntoBookmark(int index, const QString &name, const DUrl &url)
{
    BookMarkPointer bookmark(new BookMark(QDateTime::currentDateTime(), name, url));
    m_bookmarks.insert(index, bookmark);
    save();
    return bookmark;
}

void BookMarkManager::removeBookmark(BookMarkPointer bookmark)
{
    m_bookmarks.removeOne(bookmark);
    save();
}

void BookMarkManager::renameBookmark(BookMarkPointer bookmark, const QString &newname)
{
    bookmark->setName(newname);
    save();
}

void BookMarkManager::moveBookmark(int from, int to)
{
    m_bookmarks.move(from, to);
    save();
}

void BookMarkManager::reLoad()
{
    m_bookmarks.clear();
    load();
}

const QList<DAbstractFileInfoPointer> BookMarkManager::getChildren(const QSharedPointer<DFMGetChildrensEvent> &event) const
{
    const QString &frav = event->url().fragment();

    if (!frav.isEmpty()) {
        const QList<DAbstractFileInfoPointer> &list = fileService->getChildren(event->sender(), DUrl::fromLocalFile(frav),
                                                                               event->nameFilters(), event->filters(), event->flags());

        return list;
    }

    QList<DAbstractFileInfoPointer> infolist;

    for (int i = 0; i < m_bookmarks.size(); i++)
    {
        infolist.append(DAbstractFileInfoPointer(m_bookmarks.at(i)));
    }

    return infolist;
}

const DAbstractFileInfoPointer BookMarkManager::createFileInfo(const QSharedPointer<DFMCreateFileInfoEvnet> &event) const
{
    return DAbstractFileInfoPointer(new BookMark(event->url()));
}
