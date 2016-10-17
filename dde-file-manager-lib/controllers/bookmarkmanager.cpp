#include "bookmarkmanager.h"
#include "dfileservices.h"

#include "models/fileinfo.h"
#include "shutil/standardpath.h"

#include "app/define.h"

#include <QJsonObject>
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QByteArray>
#include <QDateTime>
#include <QUrl>

#include <stdlib.h>


BookMarkManager::BookMarkManager(QObject *parent)
    : DAbstractFileController(parent)
    , BaseManager()
{
    load();
    fileService->setFileUrlHandler(BOOKMARK_SCHEME, "", this);
}

BookMarkManager::~BookMarkManager()
{

}

void BookMarkManager::load()
{
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

QList<BookMark *> BookMarkManager::getBookmarks()
{
    return m_bookmarks;
}

QString BookMarkManager::cachePath()
{
    return getCachePath("bookmark");
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
        m_bookmarks.append(new BookMark(QDateTime::fromString(time), name, DUrl(url)));
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
        localArray.append(object);
    }
    json["Bookmark"] = localArray;
}

BookMark * BookMarkManager::writeIntoBookmark(int index, const QString &name, const DUrl &url)
{
    BookMark * bookmark = new BookMark(QDateTime::currentDateTime(), name, url);
    m_bookmarks.insert(index, bookmark);
    save();
    return bookmark;
}

void BookMarkManager::removeBookmark(BookMark *bookmark)
{
//    for(int i = 0; i < m_bookmarks.size(); i++)
//    {
//        BookMark * bookmark = m_bookmarks.at(i);
//        if(bookmark->getName() == name && bookmark->getUrl() == url)
//        {
//            m_bookmarks.removeOne(bookmark);
//            break;
//        }
//    }
    m_bookmarks.removeOne(bookmark);
    save();
}

void BookMarkManager::renameBookmark(BookMark* bookmark, const QString &newname)
{
    bookmark->setName(newname);
    save();
}

void BookMarkManager::moveBookmark(int from, int to)
{
    m_bookmarks.move(from, to);
    save();
}

const QList<AbstractFileInfoPointer> BookMarkManager::getChildren(const DUrl &fileUrl, const QStringList &nameFilters,
                                                                  QDir::Filters filters, QDirIterator::IteratorFlags flags,
                                                                  bool &accepted) const
{
    accepted = true;

    const QString &frav = fileUrl.fragment();

    if(!frav.isEmpty())
    {
        DUrl localUrl = DUrl::fromLocalFile(frav);

        QList<AbstractFileInfoPointer> list = fileService->getChildren(localUrl, nameFilters, filters, flags);

        return list;
    }

    QList<AbstractFileInfoPointer> infolist;

    for (int i = 0; i < m_bookmarks.size(); i++)
    {
        BookMark * bm = m_bookmarks.at(i);

        infolist.append(AbstractFileInfoPointer(new BookMark(*bm)));
    }

    return infolist;
}

const AbstractFileInfoPointer BookMarkManager::createFileInfo(const DUrl &fileUrl, bool &accepted) const
{
    accepted = true;

    return AbstractFileInfoPointer(new BookMark(fileUrl));
}
