#include "bookmarkmanager.h"
#include "fileinfo.h"


BookMarkManager::BookMarkManager(QObject *parent) : BaseManager(parent)
{
    load();
}

BookMarkManager::~BookMarkManager()
{

}

void BookMarkManager::load()
{
    //TODO: check permission and existence of the path
    QString user = getenv("USER");
    QString configPath = "/home/" + user + "/.cache/dde-file-manager/bookmark.json";
    QFile file(configPath);
    if (!file.open(QIODevice::ReadOnly))
    {
        qDebug() << "Couldn't open bookmark file!";
        return;
    }
    QByteArray data = file.readAll();
    QJsonDocument jsonDoc(QJsonDocument::fromJson(data));
    loadJson(jsonDoc.object());
}

void BookMarkManager::save()
{
    //TODO: check permission and existence of the path
    QString user = getenv("USER");
    QString configPath = "/home/" + user + "/.cache/dde-file-manager/bookmark.json";
    QFile file(configPath);
    if (!file.open(QIODevice::WriteOnly))
    {
        qDebug() << "Couldn't open bookmark file!";
        return;
    }
    QJsonObject object;
    writeJson(object);
    QJsonDocument jsonDoc(object);
    file.write(jsonDoc.toJson());
}

QList<BookMark *> BookMarkManager::getBookmarks()
{
    return m_bookmarks;
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
        m_bookmarks.append(new BookMark(QDateTime::fromString(time), name, url, this));
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
        object["u"] = m_bookmarks.at(i)->getUrl();
        localArray.append(object);
    }
    json["Bookmark"] = localArray;
}

void BookMarkManager::writeIntoBookmark(const QString &name, const QString &url)
{
    BookMark * bookmark = new BookMark(QDateTime::currentDateTime(), name, url, this);
    m_bookmarks.append(bookmark);
    save();
}

void BookMarkManager::removeBookmark(const QString &name, const QString &url)
{
    for(int i = 0; i < m_bookmarks.size(); i++)
    {
        BookMark * bookmark = m_bookmarks.at(i);
        if(bookmark->getName() == name && bookmark->getUrl() == url)
        {
            m_bookmarks.removeOne(bookmark);
            break;
        }
    }
    save();
}

void BookMarkManager::fetchFileInformation(const QString &url, int filter)
{
    const QString path = QUrl(url).path();

    QList<FileInfo*> infolist;

    for(int i = 0; i < m_bookmarks.size(); i++)
    {
        BookMark * bm = m_bookmarks.at(i);
        infolist.append(new FileInfo(bm->getUrl()));
    }

    emit updates(path, infolist);
}
